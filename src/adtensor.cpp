#include "addef.h"
#include "adtensor.h"
#include "adjet.h"
#include <onnxruntime_cxx_api.h>
#include <stdexcept>
#include <new>
#include <array>
#include <algorithm>
#include <utility>

using namespace std;
using namespace Ort;

namespace {

template<Feature (ADParticle::*begin)[0], Feature (ADParticle::*end)[0], class ADPFTensor>
void set_value_common(ADPFTensor &tensor, int64_t n, const ADJet &jet)
{
  int64_t s2 = tensor.get_shape()[2];
  int64_t np = min<int64_t>(s2, NPARTIFLOW);

  // feature copying
  for(int64_t p = 0; p < np; ++p) {
    int64_t f = 0;
    for(Feature *pf = jet.par[p].*begin; pf != jet.par[p].*end; ++pf) {
      tensor.at(n, f++, p) = *pf;
    }
  }

  // zero padding
  if(np == s2) return;
  int64_t nf = jet.par[0].*end - jet.par[0].*begin;
  for(int64_t f = 0; f < nf; ++f) {
    for(int64_t p = np; p < s2; ++p) tensor.at(n, f, p) = 0.0;
  }
}

}  // namespace

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

Value ADTensor::tensor(const MemoryInfo &meminfo, int64_t shape0)
{
  swap(shape0, shape[0]);
  Value value = tensor(meminfo);
  swap(shape0, shape[0]);
  return value;
}

ADPFTensor::ADPFTensor(int64_t nfeat)
  : ADTensor((array<int64_t, 3>{BATCH_SIZE, nfeat, NPARTIFLOW}.data()), 3)
{
  // do nothing
}

ADCFTensor::ADCFTensor()
  : ADTensor((array<int64_t, 2>{BATCH_SIZE, NRSLTCLASS}.data()), 2)
{
  // do nothing
}

void ADPFTensor::set_zero(int64_t n, int64_t p)
{
  for(int64_t f = 0; f < shape[1]; ++f) at(n, f, p) = 0;
}

void ADPFPoints::set_value(int64_t n, const ADJet &jet)
{
  set_value_common<&ADParticle::pf_points_begin, &ADParticle::pf_points_end>
    (*this, n, jet);
}

void ADPFFeatures::set_value(int64_t n, const ADJet &jet)
{
  set_value_common<&ADParticle::pf_features_begin, &ADParticle::pf_features_end>
    (*this, n, jet);
}

void ADPFVectors::set_value(int64_t n, const ADJet &jet)
{
  set_value_common<&ADParticle::pf_vectors_begin, &ADParticle::pf_vectors_end>
    (*this, n, jet);
}

void ADPFMask::set_value(int64_t n, const ADJet &jet)
{
  set_value_common<&ADParticle::pf_mask_begin, &ADParticle::pf_mask_end>
    (*this, n, jet);
}

void ADSoftmax::get_value(int64_t n, ADParT &part) const
{
  for(int64_t c = 0; c < NRSLTCLASS; ++c) {
    part.part[c] = at(n, c);
  }
}
