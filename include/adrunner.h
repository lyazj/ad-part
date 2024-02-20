#pragma once

#include "addef.h"
#include <stddef.h>
#include <memory>
#include <vector>
#include <zlib.h>

namespace Ort {

// external classes
struct Value;
struct MemoryInfo;
struct RunOptions;
struct Session;

}

// external classes
class ADTensor;
class ADPFTensor;
class ADCFTensor;

class ADRunPack {

private:
  const Ort::MemoryInfo &meminfo;
  std::vector<std::shared_ptr<ADTensor>> tensors;
  std::vector<const char *> names;
  std::vector<Ort::Value> values;  // use this when n == BATCH_SIZE
  std::vector<Ort::Value> temp_values;  // use this when n != BATCH_SIZE

public:
  ADRunPack(const std::vector<std::shared_ptr<ADTensor>> &tensors_in,
      const Ort::MemoryInfo &);

  void get_run(int64_t n,
      std::vector<const char *> **pnames, std::vector<Ort::Value> **pvalues);
  std::vector<std::shared_ptr<ADTensor>> &get_tensors() { return tensors; }

};

class ADRunner {

private:
  ADRunPack input_pack;
  ADRunPack output_pack;
  Ort::Session &session;
  const Ort::RunOptions &runopt;
  gzFile input_file;
  gzFile output_file;
  gzFile hidden_file;

public:
  ADRunner(const std::vector<std::shared_ptr<ADPFTensor>> &input_tensors,
      const std::vector<std::shared_ptr<ADCFTensor>> &output_tensors,
      const Ort::MemoryInfo &, Ort::Session &, const Ort::RunOptions &,
      gzFile input_file, gzFile output_file, gzFile hidden_file);

  int64_t run_batch();

private:
  int64_t batch_input();
  void batch_output(int64_t n);

};
