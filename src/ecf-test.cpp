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
  double pt_jet = accumulate(p4, p4 + np4, TLorentzVector()).Pt();

  vector<tuple<int, int, double>> N_q_beta = {
    {3, 2, 1.0},
    {4, 2, 1.0},
    {0, 2, 1.0},  // invalid
    {3, 0, 1.0},  // invalid
    {3, 2, 0.0},  // invalid
  };
  ECFCalculator ecf_calc(p4, np4, pt_jet, N_q_beta);
  ecf_calc.calculate();
  for(auto [N, q, beta] : N_q_beta) {
    double r = 0.0;
    ecf_calc.get_result(N, q, beta, r);
    cout << "ECF result: N=" << N << " q=" << q << " beta=" << beta
         << ": " << r << endl;
  }
  return 0;
}
