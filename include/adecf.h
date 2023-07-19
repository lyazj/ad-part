/*
 * adecf.h - Provide energy correlation functions.
 */
#pragma once

#include <cstddef>
#include <vector>
#include <tuple>
#include <map>

class TLorentzVector;

class ECFCalculatorN {
private:
  // Parameters.
  int N;
  std::vector<std::pair<int, double>> q_beta;
  std::map<std::pair<int, double>, size_t> q_beta_index;

  // Input data.
  const TLorentzVector *p4;
  int np4;
  double pt_jet;

  // Buffers.
  std::vector<int> index;
  std::vector<double> DeltaR;
  std::vector<double> result;

public:
  ECFCalculatorN(int N, const TLorentzVector *p4, int np4, double pt_jet, const std::vector<std::pair<int, double>> &q_beta);
  void calculate();
  bool get_result(int q, double beta, double &value) const;

private:
  void calculate_recursion(int i, double pt_ratio);
  void calculate_min_deltar_multiplication(double pt_ratio);
};

class ECFCalculator {
private:
  // Parameters.
  std::vector<int> N_;
  std::map<int, size_t> N_index;

  // Underlying calculators.
  std::vector<ECFCalculatorN> calcn;

public:
  ECFCalculator(const TLorentzVector *p4, int np4, double pt_jet, const std::vector<std::tuple<int, int, double>> &N_q_beta);
  void calculate();
  bool get_result(int N, int q, double beta, double &value) const;
};

class ECFRatioCalculator {
private:
  // Parameters.
  std::vector<std::tuple<int, int, double, int, int, double>> N1_q1_beta1_N2_q2_beta2;

  // Underlying calculator.
  ECFCalculator calc;

public:
  ECFRatioCalculator(const TLorentzVector *p4, int np4, double pt_jet, const std::vector<std::tuple<int, int, double, int, int, double>> &params);
  void calculate();
  bool get_result(int N1, int q1, double beta1, int N2, int q2, double beta2, double &value) const;
  std::vector<double> get_results() const;

private:
  std::vector<std::tuple<int, int, double>> unique_N_q_beta(const std::vector<std::tuple<int, int, double, int, int, double>> &);
};
