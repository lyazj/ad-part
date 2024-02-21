#include "adsamp.h"
#include <iostream>

using namespace std;

int main()
{
  ADSampler sampler;

  sampler.add_sample(30, 0.31);
  sampler.add_sample(20, 0.19);

  size_t id;
  while((id = sampler.sample())) {
    cout << id << endl;
  }

  return 0;
}
