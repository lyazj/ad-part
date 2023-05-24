#include "adtensor.h"
#include "adjet.h"
#include <onnxruntime_cxx_api.h>
#include <stdexcept>
#include <new>
#include <array>
#include <algorithm>

#ifndef BATCH_SIZE
#define BATCH_SIZE  128
#endif  /* BATCH_SIZE */

#ifndef NPARTIFLOW
#define NPARTIFLOW  128
#endif  /* NPARTIFLOW */

#ifndef NRSLTCLASS
#define NRSLTCLASS  10
#endif  /* NRSLTCLASS */

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
  int64_t np = min<int64_t>(shape[2], NPAR_PER_JET);
  for(int64_t p = 0; p < np; ++p) {
    deta(n, p) = jet.par[p].deta;
    dphi(n, p) = jet.par[p].dphi;
  }
}

void ADPFFeatures::set_value(int64_t n, const ADJet &jet)
{
  int64_t np = min<int64_t>(shape[2], NPAR_PER_JET);
  for(int64_t p = 0; p < np; ++p) {
    log_pt(n, p) = jet.par[p].log_pt;
    log_e(n, p) = jet.par[p].log_e;
    log_pt_rel(n, p) = jet.par[p].log_pt_rel;
    log_e_rel(n, p) = jet.par[p].log_e_rel;
    delta_r(n, p) = jet.par[p].delta_r;
    charge(n, p) = jet.par[p].charge;
    is_charged_hadron(n, p) = jet.par[p].is_charged_hadron;
    is_neutral_hadron(n, p) = jet.par[p].is_neutral_hadron;
    is_photon(n, p) = jet.par[p].is_photon;
    is_electron(n, p) = jet.par[p].is_electron;
    is_muon(n, p) = jet.par[p].is_muon;
    d0(n, p) = jet.par[p].d0;
    d0_err(n, p) = jet.par[p].d0_err;
    dz(n, p) = jet.par[p].dz;
    dz_err(n, p) = jet.par[p].dz_err;
    deta(n, p) = jet.par[p].deta;
    dphi(n, p) = jet.par[p].dphi;
  }
  for(int64_t p = np; p < shape[2]; ++p) set_zero(n, p);
}

void ADPFVectors::set_value(int64_t n, const ADJet &jet)
{
  int64_t np = min<int64_t>(shape[2], NPAR_PER_JET);
  for(int64_t p = 0; p < np; ++p) {
    px(n, p) = jet.par[p].px;
    py(n, p) = jet.par[p].py;
    pz(n, p) = jet.par[p].pz;
    e(n, p) = jet.par[p].e;
  }
  for(int64_t p = np; p < shape[2]; ++p) set_zero(n, p);
}

void ADPFMask::set_value(int64_t n, const ADJet &jet)
{
  int64_t np = min<int64_t>(shape[2], NPAR_PER_JET);
  for(int64_t p = 0; p < np; ++p) {
    mask(n, p) = jet.par[p].mask;
  }
  for(int64_t p = np; p < shape[2]; ++p) set_zero(n, p);
}
