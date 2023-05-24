#include "addef.h"
#include "adtensor.h"
#include <onnxruntime_cxx_api.h>
#include <vector>

using namespace std;
using namespace Ort;

int main()
{
  // 运行环境
  Env env;
  Session session(env, "part.onnx", SessionOptions(nullptr));
  MemoryInfo memory_info = MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

  // 输入张量
  vector<const char *> input_names = {"pf_points", "pf_features", "pf_vectors", "pf_mask"};
  vector<int64_t> input_shapes = {2, 17, 4, 1};
  vector<ADPFTensor> input_tensors;
  vector<Value> input_values;
  for(size_t i = 0; i < input_shapes.size(); ++i) {
    input_tensors.emplace_back(input_shapes[i]);
    input_values.emplace_back(input_tensors[i].tensor(memory_info));
  }

  // TODO 填写输入

  // 输出张量
  vector<const char *> output_names = {"softmax"};
  vector<int64_t> output_shapes = {2};
  vector<ADCCTensor> output_tensors;
  vector<Value> output_values;
  for(size_t i = 0; i < output_shapes.size(); ++i) {
    output_tensors.emplace_back(output_shapes[i]);
    output_values.emplace_back(output_tensors[i].tensor(memory_info));
  }

  // 运行模型
  RunOptions run_options;
  session.Run(run_options,
      input_names.data(), input_values.data(), input_names.size(),
      output_names.data(), output_values.data(), output_names.size());

  // TODO 输出结果

  return 0;
}
