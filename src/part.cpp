#include "addef.h"
#include "adtensor.h"
#include "adjet.h"
#include <onnxruntime_cxx_api.h>
#include <vector>
#include <memory>

using namespace std;
using namespace Ort;

int main(int argc, char *argv[])
{
  // 运行环境
  Env env;
  Session session(env, "part.onnx", SessionOptions(nullptr));
  MemoryInfo memory_info = MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

  // 输入张量
  vector<const char *> input_names;
  vector<Value> input_values;
  vector<shared_ptr<ADPFTensor>> input_tensors;
  input_tensors.emplace_back(new ADPFFeatures);
  input_tensors.emplace_back(new ADPFVectors);
  input_tensors.emplace_back(new ADPFMask);
  for(const auto &tensor : input_tensors) {
    input_names.push_back(tensor->get_name());
    input_values.push_back(tensor->tensor(memory_info));
  }

  // TODO 填写输入

  // 输出张量
  vector<const char *> output_names;
  vector<Value> output_values;
  vector<shared_ptr<ADCFTensor>> output_tensors;
  output_tensors.emplace_back(new ADSoftmax);
  for(const auto &tensor : output_tensors) {
    output_names.push_back(tensor->get_name());
    output_values.push_back(tensor->tensor(memory_info));
  }

  // 运行模型
  RunOptions run_options;
  session.Run(run_options,
      input_names.data(), input_values.data(), input_names.size(),
      output_names.data(), output_values.data(), output_names.size());

  // TODO 输出结果

  return 0;
}
