#include "adecf.h"
#include <TLorentzVector.h>
#include <string>
#include <algorithm>
#include <utility>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <set>

using namespace std;

#ifdef ECF_DEBUG
#define ECF_INFO
#endif  /* ECF_DEBUG */

namespace {

void ecf_invalid_argument(const char *name)
{
  cerr << "WARNING: argument " << name << " out of range, ECF request refused" << endl;
}

bool validate_N_q_beta(int N, int q, double beta, int np4)
{
  if(N <= 0 || N > np4) { ecf_invalid_argument("N"); return false; }
  if(q <= 0 || q > N * (N - 1) / 2) { ecf_invalid_argument("q"); return false; }
  if(beta <= 0) { ecf_invalid_argument("beta"); return false; }
  return true;
}

}  // namespace

ECFCalculatorN::ECFCalculatorN(int N_in, const TLorentzVector *p4_in, int np4_in, double pt_jet_in, const vector<pair<int, double>> &q_beta_in)
    : N(N_in), p4(p4_in), np4(np4_in), pt_jet(pt_jet_in), index(N), DeltaR(N * (N - 1) / 2)
{
  for(auto [q, beta] : q_beta_in) {
    // Argument validation.
    bool valid = validate_N_q_beta(N, q, beta, np4);
#ifdef ECF_INFO
    clog << "ECF request: N=" << N << " q=" << q << " beta=" << beta
         << ": valid=" << boolalpha << valid << endl;
#endif  /* ECF_INFO */
    if(!valid) continue;

    // Duplication elimination.
    if(q_beta_index.emplace(make_pair(q, beta), q_beta.size()).second) {
      q_beta.push_back(make_pair(q, beta));
    }
  }
}

void ECFCalculatorN::calculate()
{
  if(q_beta.empty()) return;
  result.assign(q_beta.size(), 0.0);  // pre-sum zeroing
  calculate_recursion(0, 1.0);
}

bool ECFCalculatorN::get_result(int q, double beta, double &value) const
{
  auto it = q_beta_index.find({q, beta});
  if(it == q_beta_index.end()) return false;
  value = result[it->second];
  return true;
}

void ECFCalculatorN::calculate_recursion(int i, double pt_ratio)
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

void ECFCalculatorN::calculate_min_deltar_multiplication(double pt_ratio)
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

ECFCalculator::ECFCalculator(const TLorentzVector *p4, int np4, double pt_jet, const vector<tuple<int, int, double>> &N_q_beta_in)
{
  vector<vector<pair<int, double>>> q_beta;

  // Unique and group parameter tuples by N.
  for(auto [N, q, beta] : N_q_beta_in) {
    auto N_it = N_index.find(N);
    if(N_it == N_index.end()) {  // new N
      N_index.emplace(N, N_.size());
      N_.push_back(N);
      q_beta.push_back({{q, beta}});
    } else {  // existing N
      q_beta[N_it->second].emplace_back(make_pair(q, beta));
    }
  }

  // Initialize underlying calculators.
  calcn.reserve(N_.size());
  for(size_t i = 0; i < N_.size(); ++i) {
    calcn.emplace_back(N_[i], p4, np4, pt_jet, q_beta[i]);
  }
}

void ECFCalculator::calculate()
{
  for(ECFCalculatorN &cn : calcn) cn.calculate();
}

bool ECFCalculator::get_result(int N, int q, double beta, double &value) const
{
  auto it = N_index.find(N);
  if(it == N_index.end()) return false;
  return calcn[it->second].get_result(q, beta, value);
}

vector<tuple<int, int, double>> ECFRatioCalculator::unique_N_q_beta(const vector<tuple<int, int, double, int, int, double>> &params)
{
  set<tuple<int, int, double>> N_q_beta_set;
  for(auto [N1, q1, beta1, N2, q2, beta2] : params) {
    N_q_beta_set.emplace(N1, q1, beta1);
    N_q_beta_set.emplace(N2, q2, beta2);
  }
  return {N_q_beta_set.begin(), N_q_beta_set.end()};
}

ECFRatioCalculator::ECFRatioCalculator(const TLorentzVector *p4, int np4, double pt_jet, const vector<tuple<int, int, double, int, int, double>> &params)
  : N1_q1_beta1_N2_q2_beta2(params), calc(p4, np4, pt_jet, unique_N_q_beta(params))
{
  // Empty.
}

void ECFRatioCalculator::calculate()
{
  calc.calculate();
}

bool ECFRatioCalculator::get_result(int N1, int q1, double beta1, int N2, int q2, double beta2, double &value) const
{
  double r1, r2;
  if(!calc.get_result(N1, q1, beta1, r1) || !calc.get_result(N2, q2, beta2, r2)) return false;
  value = r1 / pow(r2, (q1 * beta1) / (q2 * beta2));
  return true;
}

vector<double> ECFRatioCalculator::get_results() const
{
  vector<double> results;
  results.reserve(N1_q1_beta1_N2_q2_beta2.size());
  for(auto [N1, q1, beta1, N2, q2, beta2] : N1_q1_beta1_N2_q2_beta2) {
    double r = 0.0;  // zero padding on out-of-range error
    get_result(N1, q1, beta1, N2, q2, beta2, r);
    results.push_back(r);
  }
  return results;
}
