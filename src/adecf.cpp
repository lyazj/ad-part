#include "adecf.h"
#include <TLorentzVector.h>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <utility>
#include <cmath>
#include <iostream>
#include <iomanip>

using namespace std;

#ifdef ECF_DEBUG
#define ECF_INFO
#endif  /* ECF_DEBUG */

namespace {

class ECFCalculator {
private:
  // Parameters.
  int N;
  const vector<pair<int, double>> &q_beta;

  // Input data.
  const TLorentzVector *p4;
  int np4;
  double pt_jet;

  // Buffers.
  vector<int> index;
  vector<double> DeltaR;
  vector<double> result;

public:
  ECFCalculator(int N, const TLorentzVector *p4, int np4, double pt_jet, const vector<pair<int, double>> &q_beta);
  const vector<double> &calculate();

private:
  void calculate_recursion(int i, double pt_ratio);
  void calculate_min_deltar_multiplication(double pt_ratio);
};

ECFCalculator::ECFCalculator(int N_in, const TLorentzVector *p4_in, int np4_in, double pt_jet_in, const vector<pair<int, double>> &q_beta_in)
    : N(N_in), q_beta(q_beta_in), p4(p4_in), np4(np4_in), pt_jet(pt_jet_in), index(N), DeltaR(N * (N - 1) / 2)
{
  // empty
}

const vector<double> &ECFCalculator::calculate()
{
  result.assign(q_beta.size(), 0.0);  // pre-sum zeroing
  calculate_recursion(0, 1.0);
  return result;
}

void ECFCalculator::calculate_recursion(int i, double pt_ratio)
{
  // Recursion exit.
  if(i == N) return calculate_min_deltar_multiplication(pt_ratio);

  // Recursion.
  int k = i == 0 ? 0 : index[i - 1] + 1;
  for(; k <= np4 - N + i; ++k) {
    index[i] = k;
    calculate_recursion(i + 1, pt_ratio * (p4[k].Pt() / pt_jet));
  }
}

void ECFCalculator::calculate_min_deltar_multiplication(double pt_ratio)
{
  // Iterate over permutations.
  int k = 0;
  for(int i = 0; i < N - 1; ++i) {
    for(int j = i + 1; j < N; ++j) {
      DeltaR[k++] = p4[index[i]].DeltaR(p4[index[j]]);
    }
  }
  sort(DeltaR.begin(), DeltaR.end());

  // Compute result for each (q, beta) pair.
  size_t p = 0;
  for(auto [q, beta] : q_beta) {
    double r = 1.0;
    for(int i = 0; i < q; ++i) r *= pow(DeltaR[i], beta);

#ifdef ECF_DEBUG
    clog << defaultfloat << "ECF candidate (q=" << q << " beta=" << beta << "):";
    for(int j = 0; j < N; ++j) clog << " " << index[j];
    clog << ":";
    for(int i = 0; i < k; ++i) clog << " " << fixed << setprecision(3) << DeltaR[i];
    clog << ": " << r << endl;
#endif  /* ECF_DEBUG */

    result[p++] += pt_ratio * r;
  }
}

void ecf_invalid_argument(const char *name)
{
  throw std::invalid_argument("argument "s + name + " out of range");
}

}  // namespace

vector<double> ecf(int N, const TLorentzVector *p4, int np4, double pt_jet, const vector<pair<int, double>> &q_beta)
{
  for(auto [q, beta] : q_beta) {
#ifdef ECF_INFO
    clog << "ECF call: q=" << q << " N=" << N << " beta=" << beta << " np4=" << np4 << endl;
#endif  /* ECF_INFO */

    // Perform argument validation.
    if(N <= 0 || N > np4) ecf_invalid_argument("N");
    if(q <= 0 || q > N * (N - 1) / 2) ecf_invalid_argument("q");
    if(beta <= 0) ecf_invalid_argument("beta");
  }

  return ECFCalculator(N, p4, np4, pt_jet, q_beta).calculate();
}

double ecf(int N, const TLorentzVector *p4, int np4, double pt_jet, int q, double beta)
{
  return ecf(N, p4, np4, pt_jet, {{q, beta}})[0];
}

double N2(double beta, const TLorentzVector *p4, int np4, double pt_jet)
{
  return ecf(3, p4, np4, pt_jet, 2, beta) / ecf(2, p4, np4, pt_jet, 1, beta);
}

double N3(double beta, const TLorentzVector *p4, int np4, double pt_jet)
{
  return ecf(4, p4, np4, pt_jet, 2, beta) / ecf(3, p4, np4, pt_jet, 1, beta);
}
