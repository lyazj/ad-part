#include "adecf.h"
#include <TLorentzVector.h>
#include <string>
#include <stdexcept>
#include <algorithm>
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
  int q;
  int N;
  double beta;

  // Input data.
  const TLorentzVector *p4;
  int np4;
  double pt_jet;

  // Buffers.
  int *index;
  double *DeltaR;

public:
  ECFCalculator(int q, int N, double beta, const TLorentzVector *p4, int np4, double pt_jet);
  ~ECFCalculator();
  double calculate() const;
  double calculate_recursion(int i, double pt_ratio) const;
  double calculate_min_deltar_multiplication() const;
};

ECFCalculator::ECFCalculator(int q_in, int N_in, double beta_in,
    const TLorentzVector *p4_in, int np4_in, double pt_jet_in)
    : q(q_in), N(N_in), beta(beta_in), p4(p4_in), np4(np4_in), pt_jet(pt_jet_in)
{
  // Allocate buffers.
  index = NULL;
  DeltaR = NULL;
  try {
    index = new int[N];
    DeltaR = new double[N * (N - 1) / 2];
  } catch(const bad_alloc &) {
    delete[] index;
    delete[] DeltaR;
    throw;
  }
}

ECFCalculator::~ECFCalculator()
{
  // Deallocate buffers.
  delete[] index;
  delete[] DeltaR;
}

double ECFCalculator::calculate() const
{
  return calculate_recursion(0, 1.0);
}

double ECFCalculator::calculate_recursion(int i, double pt_ratio) const
{
  // Recursion exit.
  if(i == N) return pt_ratio * calculate_min_deltar_multiplication();

  // Recursion.
  double r = 0.0;
  int k = i == 0 ? 0 : index[i - 1] + 1;
  for(; k <= np4 - N + i; ++k) {
    index[i] = k;
    r += calculate_recursion(i + 1, pt_ratio * (p4[k].Pt() / pt_jet));
  }
  return r;
}

double ECFCalculator::calculate_min_deltar_multiplication() const
{
  // Iterate over permutations.
  int k = 0;
  for(int i = 0; i < N - 1; ++i) {
    for(int j = i + 1; j < N; ++j) {
      DeltaR[k++] = p4[index[i]].DeltaR(p4[index[j]]);
    }
  }

  // Multiplicate q minimal values.
  sort(DeltaR, DeltaR + k);
  double r = 1.0;
  for(int i = 0; i < q; ++i) r *= pow(DeltaR[i], beta);

#ifdef ECF_DEBUG
  clog << "ECF candidate:";
  for(int j = 0; j < N; ++j) clog << " " << index[j];
  clog << ":";
  for(int i = 0; i < k; ++i) clog << " " << fixed << setprecision(3) << DeltaR[i];
  clog << ": " << r << endl;
#endif  /* ECF_DEBUG */

  return r;
}

void ecf_invalid_argument(const char *name)
{
  throw std::invalid_argument("argument "s + name + " out of range");
}

}  // namespace

double ecf(int q, int N, double beta, const TLorentzVector *p4, int np4, double pt_jet)
{
#ifdef ECF_INFO
  clog << "ECF call: q=" << q << " N=" << N << " beta=" << beta << " np4=" << np4 << endl;
#endif  /* ECF_INFO */

  // Perform argument validation.
  if(N <= 0 || N > np4) ecf_invalid_argument("N");
  if(q <= 0 || q > N * (N - 1) / 2) ecf_invalid_argument("q");
  if(beta <= 0) ecf_invalid_argument("beta");

  return ECFCalculator(q, N, beta, p4, np4, pt_jet).calculate();
}

double N2(double beta, const TLorentzVector *p4, int np4, double pt_jet)
{
  double e_2_3_beta = ecf(2, 3, beta, p4, np4, pt_jet);
  double e_1_2_beta = ecf(1, 2, beta, p4, np4, pt_jet);
  return e_2_3_beta / (e_1_2_beta * e_1_2_beta);
}

double N3(double beta, const TLorentzVector *p4, int np4, double pt_jet)
{
  double e_2_4_beta = ecf(2, 4, beta, p4, np4, pt_jet);
  double e_1_3_beta = ecf(1, 3, beta, p4, np4, pt_jet);
  return e_2_4_beta / (e_1_3_beta * e_1_3_beta);
}
