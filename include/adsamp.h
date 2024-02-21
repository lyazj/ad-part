#pragma once
#include <stddef.h>

// Sampling without replacement.
class ADSampler {

private:
  class Driver;
  Driver *driver;

public:
  ADSampler();
  ~ADSampler();
  ADSampler(const ADSampler &) = delete;

  size_t add_sample(size_t n, double w);
  size_t sample();
};
