#pragma once

#include "addef.h"
#include <stdint.h>

namespace Ort {

// external classes
struct Value;
struct MemoryInfo;

}

// external classes
class ADJet;
class ADParT;

class ADTensor {

protected:
  Feature *data;
  int64_t size;
  int64_t *shape;
  int64_t shape_size;

public:
  ADTensor(const int64_t *shape_in, int64_t shape_size_in);
  ADTensor(ADTensor &&) noexcept;
  virtual ~ADTensor();
  
  Ort::Value tensor(const Ort::MemoryInfo &);
  Ort::Value tensor(const Ort::MemoryInfo &, int64_t shape0);
  Feature *get_data() { return data; }
  int64_t get_size() { return size; }
  const int64_t *get_shape() { return shape; }
  int64_t get_shape_size() { return shape_size; }
  virtual const char *get_name() { return "tensor"; }

};

class ADPFTensor : public ADTensor {

public:
  ADPFTensor(int64_t nfeat);
  virtual const char *get_name() override = 0;

  const Feature &at(int64_t n, int64_t f, int64_t p) const {
    return data[(n * shape[1] + f) * shape[2] + p];
  }
  Feature &at(int64_t n, int64_t f, int64_t p) {
    return (Feature &)((const ADPFTensor *)this)->at(n, f, p);
  }

  void set_zero(int64_t n, int64_t p);
  virtual void set_value(int64_t n, const ADJet &) = 0;

};

class ADCFTensor : public ADTensor {

public:
  ADCFTensor();
  virtual const char *get_name() override = 0;

  const Feature &at(int64_t n, int64_t c) const {
    return data[(n * shape[1]) + c];
  }
  Feature &at(int64_t n, int64_t c) {
    return (Feature &)((const ADCFTensor *)this)->at(n, c);
  }

  virtual void get_value(int64_t n, ADParT &) const = 0;

};

class ADPFPoints : public ADPFTensor {

public:
  ADPFPoints() : ADPFTensor(2) { }
  virtual const char *get_name() override { return "pf_points"; }

  Feature &deta(int64_t n, int64_t p) { return at(n, 0, p); }
  Feature &dphi(int64_t n, int64_t p) { return at(n, 1, p); }

  virtual void set_value(int64_t n, const ADJet &) override;

};

class ADPFFeatures : public ADPFTensor {

public:
  ADPFFeatures() : ADPFTensor(17) { }
  virtual const char *get_name() override { return "pf_features"; }

  Feature &log_pt(int64_t n, int64_t p)            { return at(n,  0, p); }
  Feature &log_e(int64_t n, int64_t p)             { return at(n,  1, p); }
  Feature &log_pt_rel(int64_t n, int64_t p)        { return at(n,  2, p); }
  Feature &log_e_rel(int64_t n, int64_t p)         { return at(n,  3, p); }
  Feature &delta_r(int64_t n, int64_t p)           { return at(n,  4, p); }
  Feature &charge(int64_t n, int64_t p)            { return at(n,  5, p); }
  Feature &is_charged_hadron(int64_t n, int64_t p) { return at(n,  6, p); }
  Feature &is_neutral_hadron(int64_t n, int64_t p) { return at(n,  7, p); }
  Feature &is_photon(int64_t n, int64_t p)         { return at(n,  8, p); }
  Feature &is_electron(int64_t n, int64_t p)       { return at(n,  9, p); }
  Feature &is_muon(int64_t n, int64_t p)           { return at(n, 10, p); }
  Feature &d0(int64_t n, int64_t p)                { return at(n, 11, p); }
  Feature &d0_err(int64_t n, int64_t p)            { return at(n, 12, p); }
  Feature &dz(int64_t n, int64_t p)                { return at(n, 13, p); }
  Feature &dz_err(int64_t n, int64_t p)            { return at(n, 14, p); }
  Feature &deta(int64_t n, int64_t p)              { return at(n, 15, p); }
  Feature &dphi(int64_t n, int64_t p)              { return at(n, 16, p); }

  virtual void set_value(int64_t n, const ADJet &) override;

};

class ADPFVectors : public ADPFTensor {

public:
  ADPFVectors() : ADPFTensor(4) { }
  virtual const char *get_name() override { return "pf_vectors"; }

  Feature &px(int64_t n, int64_t p) { return at(n, 0, p); }
  Feature &py(int64_t n, int64_t p) { return at(n, 1, p); }
  Feature &pz(int64_t n, int64_t p) { return at(n, 2, p); }
  Feature &e(int64_t n, int64_t p)  { return at(n, 3, p); }

  virtual void set_value(int64_t n, const ADJet &) override;

};

class ADPFMask : public ADPFTensor {

public:
  ADPFMask() : ADPFTensor(1) { }
  virtual const char *get_name() override { return "pf_mask"; }

  Feature &mask(int64_t n, int64_t p) { return at(n, 0, p); }

  virtual void set_value(int64_t n, const ADJet &) override;

};

class ADSoftmax : public ADCFTensor {

public:
  ADSoftmax() : ADCFTensor() { }
  virtual const char *get_name() override { return "softmax"; }

  Feature &softmax(int64_t n, int64_t c) { return at(n, c); }

  virtual void get_value(int64_t n, ADParT &) const override;

};
