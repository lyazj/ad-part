#include "addef.h"
#include "adenv.h"
#include "adtensor.h"
#include "adrunner.h"
#include <onnxruntime_cxx_api.h>
#include <vector>
#include <memory>
#include <stdio.h>
#include <zlib.h>

using namespace std;
using namespace Ort;

int main(int argc, char *argv[])
{
  // 强制行缓冲输出
  setlbf();

  // 运行参数
  if(argc != 3) {
    fprintf(stderr, "usage: %s <dumpfile> <partfile>\n",
        get_invoc_short_name());
    return 1;
  }
  const char *dumpfile = argv[1];  // dumpfile to read
  const char *partfile = argv[2];  // partfile to write

  // 输入输出文件
  gzFile dump = gzopen(dumpfile, "rb");
  if(dump == NULL) {
    fprintf(stderr, "ERROR: error opening dumpfile\n");
    return 1;
  }
  shared_ptr<gzFile_s> dump_guard(dump, [](gzFile f) { gzclose(f); } );
  gzFile part = gzopen(partfile, "wb");
  if(part == NULL) {
    fprintf(stderr, "ERROR: error opening partfile\n");
    return 1;
  }
  shared_ptr<gzFile_s> part_guard(part, [](gzFile f) { gzclose(f); } );

  // 运行环境
  Env env;
  Session session(env, "part.onnx", SessionOptions(nullptr));
  MemoryInfo memory_info = MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

  // 输入张量
  vector<shared_ptr<ADPFTensor>> input;
  input.emplace_back(new ADPFFeatures);
  input.emplace_back(new ADPFVectors);
  input.emplace_back(new ADPFMask);

  // 输出张量
  vector<shared_ptr<ADCFTensor>> output;
  output.emplace_back(new ADSoftmax);

  // 运行模型，读取输入，写入输出
  RunOptions options;
  ADRunner runner(input, output, memory_info, session, options, dump, part);
  int64_t b = 0;
  int64_t n;
  do {
    n = runner.run_batch();
    printf("---------- batch %lld ----------\n", (long long)b++);
    printf("%lld jets processed\n", (long long)n);
  } while(n);

  return 0;
}
