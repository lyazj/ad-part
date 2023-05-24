#include "adtensor.h"
#include <onnxruntime_cxx_api.h>
#include <stdexcept>
#include <new>
#include <array>

using namespace std;
using namespace Ort;

ADTensor::ADTensor(const int64_t *shape_in, int64_t shape_size_in)
{
  int64_t s = 1;
  for(int64_t i = 0; i < shape_size_in; ++i) {
    s *= shape_in[i];
  }

  data = (Feature *)malloc(s * sizeof(Feature));
  if(data == NULL) throw bad_alloc();
  shape = (int64_t *)malloc(shape_size_in * sizeof(int64_t));
  if(shape == NULL) { free(data); throw bad_alloc(); }

  size = s;
  for(int64_t i = 0; i < shape_size_in; ++i) {
    shape[i] = shape_in[i];
  }
  shape_size = shape_size_in;
}

ADTensor::ADTensor(ADTensor &&tensor) noexcept
{
  data = tensor.data;
  size = tensor.size;
  shape = tensor.shape;
  shape_size = tensor.shape_size;
  tensor.data = NULL;
  tensor.shape = NULL;
}

ADTensor::~ADTensor()
{
  free(shape);
  free(data);
}

Value ADTensor::tensor(const MemoryInfo &meminfo)
{
  return Value::CreateTensor(meminfo, data, size, shape, shape_size);
}

ADPFTensor::ADPFTensor(int64_t nfeat)
  : ADTensor((array<int64_t, 3>{BATCH_SIZE, nfeat, NPARTIFLOW}.data()), 3)
{
  // do nothing
}

ADCCTensor::ADCCTensor(int64_t nclss)
  : ADTensor((array<int64_t, 2>{BATCH_SIZE, nclss}.data()), 2)
{
  // do nothing
}
