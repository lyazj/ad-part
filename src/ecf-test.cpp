#include "adecf.h"
#include <TLorentzVector.h>
#include <numeric>
#include <iostream>

using namespace std;

int main()
{
  constexpr int np4 = 6;
  TLorentzVector p4[np4];
  p4[0].SetPtEtaPhiE(100, 0, 0, 1e10);
  p4[1].SetPtEtaPhiE(100, 1, 0, 1e10);
  p4[2].SetPtEtaPhiE(100, 2, 0, 1e10);
  p4[3].SetPtEtaPhiE(100, 3, 0, 1e10);
  p4[4].SetPtEtaPhiE(100, 4, 0, 1e10);
  p4[5].SetPtEtaPhiE(100, 5, 0, 1e10);
  cout << ecf(3, p4, np4, accumulate(p4, p4 + np4, TLorentzVector()).Pt(), 2, 1) << endl;
  cout << ecf(4, p4, np4, accumulate(p4, p4 + np4, TLorentzVector()).Pt(), 2, 1) << endl;
  return 0;
}
