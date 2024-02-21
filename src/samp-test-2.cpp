#include "adsamp.h"
#include <iostream>

using namespace std;

int main()
{
  ADSampler sampler;

  sampler.add_sample(1000000, 0.31);
  sampler.add_sample(1000000, 0.19);
  sampler.add_sample(1000000, 0.50);

  int cnt[3] = { };

  for(size_t i = 0; i < 100000; ++i) {
    ++cnt[sampler.sample() - 1];
  }
  cout << cnt[0] << ' ' << cnt[1] << ' ' << cnt[2] << endl;

  for(size_t i = 100000; i < 3000000; ++i) {
    ++cnt[sampler.sample() - 1];
  }
  cout << cnt[0] << ' ' << cnt[1] << ' ' << cnt[2] << endl;

  return 0;
}
