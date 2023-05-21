#include <onnxruntime_cxx_api.h>
#include <vector>
#include <utility>
#include <iostream>
#include <cstdint>
#include <cstring>

using namespace std;
using namespace Ort;

constexpr int64_t BATCH_SIZE = 128;
constexpr int64_t INPUT_DIM = 64;
constexpr int64_t OUTPUT_DIM = 1;

int main()
{
  // 运行环境
  Env env;
  SessionOptions options;
  OrtSessionOptionsAppendExecutionProvider_CUDA(options, 0);
  Session session(env, "test-model.onnx", options);
  MemoryInfo memory_info = MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

  // 输入张量
  const char *input_names[] = {"input_layer"};
  float input_data[BATCH_SIZE * INPUT_DIM] = { };
  int64_t input_shape[2] = {BATCH_SIZE, INPUT_DIM};
  Value input_tensor = Value::CreateTensor<float>(memory_info, input_data, BATCH_SIZE * INPUT_DIM, input_shape, 2);

  // TODO 填写输入

  // 输出张量
  const char *output_names[] = {"output_layer"};
  float output_data[BATCH_SIZE * OUTPUT_DIM] = { };
  int64_t output_shape[2] = {BATCH_SIZE, OUTPUT_DIM};
  Value output_tensor = Value::CreateTensor<float>(memory_info, output_data, BATCH_SIZE * OUTPUT_DIM, output_shape, 2);

  // 运行模型
  RunOptions run_options;
  session.Run(run_options, input_names, &input_tensor, 1, output_names, &output_tensor, 1);

  // 输出结果
  for(int64_t i = 0; i < BATCH_SIZE; ++i) {
    cout << output_data[i] << endl;
  }

  return 0;
}
