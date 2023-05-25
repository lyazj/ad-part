#include "adrunner.h"
#include "adtensor.h"
#include "adjet.h"
#include <onnxruntime_cxx_api.h>

using namespace std;

ADRunPack::ADRunPack(const vector<shared_ptr<ADTensor>> &tensors_in,
    const Ort::MemoryInfo &meminfo_in) : meminfo(meminfo_in)
{
  size_t n = tensors_in.size();
  tensors.reserve(n);
  names.reserve(n);
  values.reserve(n);
  temp_values.reserve(n);

  for(const auto &tensor : tensors_in) {
    tensors.push_back(tensor);
    names.push_back(tensor->get_name());
    values.push_back(tensor->tensor(meminfo));
  }
}

void ADRunPack::get_run(int64_t n,
    vector<const char *> **pnames, vector<Ort::Value> **pvalues)
{
  *pnames = &names;
  if(n == BATCH_SIZE) {
    *pvalues = &values;
  } else {
    temp_values.clear();
    for(const auto &tensor : tensors) {
      temp_values.push_back(tensor->tensor(meminfo, n));
    }
    *pvalues = &temp_values;
  }
}

ADRunner::ADRunner(const vector<shared_ptr<ADPFTensor>> &input_tensors,
    const vector<shared_ptr<ADCFTensor>> &output_tensors,
    const Ort::MemoryInfo &meminfo, Ort::Session &session_in,
    const Ort::RunOptions &runopt_in, gzFile input_file_in, gzFile output_file_in)
  : input_pack({input_tensors.begin(), input_tensors.end()}, meminfo),
    output_pack({output_tensors.begin(), output_tensors.end()}, meminfo),
    session(session_in), runopt(runopt_in),
    input_file(input_file_in), output_file(output_file_in)
{
  // do nothing
}

int64_t ADRunner::run_batch()
{
  int64_t n = batch_input();
  if(n == 0) return 0;

  vector<const char *> *input_names;
  vector<Ort::Value> *input_values;
  input_pack.get_run(n, &input_names, &input_values);

  vector<const char *> *output_names;
  vector<Ort::Value> *output_values;
  output_pack.get_run(n, &output_names, &output_values);

  session.Run(runopt,
      input_names->data(), input_values->data(), input_names->size(),
      output_names->data(), output_values->data(), output_names->size());

  batch_output(n);
  return n;
}

int64_t ADRunner::batch_input()
{
  int64_t n = 0;
  ADJet jet;
  while(n < BATCH_SIZE) {
    if(!jet.read(input_file)) break;
    for(const auto &tensor : input_pack.get_tensors()) {
      ((ADPFTensor *)tensor.get())->set_value(n, jet);
    }
    ++n;
  }
  return n;
}

void ADRunner::batch_output(int64_t n)
{
  ADParT part;
  for(int64_t i = 0; i < n; ++i) {
    for(const auto &tensor : output_pack.get_tensors()) {
      ((ADCFTensor *)tensor.get())->get_value(i, part);
    }
    part.write(output_file);
  }
}
