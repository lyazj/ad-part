#include "adsamp.h"
#include <vector>
#include <random>
#include <iostream>

using namespace std;

class ADSampler::Driver {
public:
  Driver();
  size_t add_sample(size_t n, double w);
  size_t sample();

private:
  // Sample number and weight.
  // num[0] := 0, reserved
  vector<size_t> num;
  size_t num_sum;
  vector<double> wgt;

  // Binary indexed tree.
  // wgt_bit[0] := 0, reserved
  // wgt_bit[i] := wgt_sum[i] - wgt_sum[i - (i & -i)]
  vector<double> wgt_bit;

  // Random number generator.
  default_random_engine re;
  uniform_real_distribution<double> urd;

  // Helper routines.
  double rand(double maxval);
  double get_wgt_presum(size_t i) const;
  size_t upper_bound(double val) const;
  size_t adjust_sample(size_t i) const;
  void draw_sample(size_t i);
};

ADSampler::Driver::Driver() : num{0}, num_sum(0), wgt{0.0}, wgt_bit{0.0}
{
  // empty
}

size_t ADSampler::Driver::add_sample(size_t n, double w)
{
  size_t i = num.size();
  num.push_back(n);
  num_sum += n;
  wgt.push_back(w);

  // Let wgt_bit[i] := wgt_sum[i] - wgt_sum[l].
  w *= n;
  for(size_t l = i ^ (i & -i), j = i - 1; j != l; j ^= j & -j) {
    w += wgt_bit[j];
  }
  wgt_bit.push_back(w);

#ifdef SAMP_DEBUG
  clog << "DEBUG: " << __func__ << ": wgt_sum:";
  for(size_t j = 1; j < num.size(); ++j) {
    clog << ' ' << get_wgt_presum(j);
  }
  clog << endl;
#endif  /* SAMP_DEBUG */

  return i;  // sample set identifier
}

size_t ADSampler::Driver::sample()
{
  if(num_sum == 0) return 0;  // invalid identifier

  double wgt_sum = get_wgt_presum(num.size() - 1);
  double rndnum = rand(wgt_sum);
  size_t i = upper_bound(rndnum);
  i = adjust_sample(i);
  draw_sample(i);

#ifdef SAMP_DEBUG
  clog << "DEBUG: " << __func__ << ": wgt_sum:";
  for(size_t j = 1; j < num.size(); ++j) {
    clog << ' ' << get_wgt_presum(j);
  }
  clog << endl;
#endif  /* SAMP_DEBUG */

  return i;
}

double ADSampler::Driver::rand(double maxval)
{
  return urd(re) * maxval;
}

double ADSampler::Driver::get_wgt_presum(size_t i) const
{
  double w = wgt_bit[i];
  while(i ^= i & -i) {
    w += wgt_bit[i];
  }
  return w;
}

size_t ADSampler::Driver::upper_bound(double val) const
{
  size_t l = 0, r = wgt_bit.size();
  while(l != r) {  // Invariant: wgt_sum[r] > val.
    size_t m = (l + r) >> 1;
    double w = get_wgt_presum(m);
    if(w <= val) {  // Drop [l, m].
      l = m + 1;
    } else {  // Drop [m, r].
      r = m;
    }
  }
  return r;
}

size_t ADSampler::Driver::adjust_sample(size_t i) const
{
  if(num[i]) return i;
  cerr << "WARNING: " << __func__ << ": adjusting sample to draw" << endl;

  size_t n = num.size();
  for(size_t j = i + 1; j < n; ++j) {
    if(num[j]) return j;
  }
  if(i) for(size_t j = i - 1; j; --j) {  // j == 0 is reserved.
    if(num[j]) return j;
  }
  abort();  // Control never reaches here.
}

void ADSampler::Driver::draw_sample(size_t i)
{
  --num[i];
  --num_sum;
  double w = wgt[i];
  for(size_t n = wgt_bit.size(); i < n; i += i & -i) {
    wgt_bit[i] -= w;
  }
}

ADSampler::ADSampler()
{
  driver = new Driver;
}

ADSampler::~ADSampler()
{
  delete driver;
}

size_t ADSampler::add_sample(size_t n, double w)
{
  return driver->add_sample(n, w);
}

size_t ADSampler::sample()
{
  return driver->sample();
}
