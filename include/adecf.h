/*
 * adecf.h - Provide energy correlation functions.
 */
#pragma once

#include <vector>
#include <utility>

class TLorentzVector;

std::vector<double> ecf(int N, const TLorentzVector *p4, int np4, double pt_jet, const std::vector<std::pair<int, double>> &q_beta);
double ecf(int N, const TLorentzVector *p4, int np4, double pt_jet, int q, double beta);

double N2(double beta, const TLorentzVector *p4, int np4, double pt_jet);
double N3(double beta, const TLorentzVector *p4, int np4, double pt_jet);
