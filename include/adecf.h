/*
 * ecf - Provide energy correlation functions.
 */
#pragma once

class TLorentzVector;

double ecf(int q, int N, double beta, const TLorentzVector *p4, int np4, double pt_jet);

double N2(double beta, const TLorentzVector *p4, int np4, double pt_jet);
double N3(double beta, const TLorentzVector *p4, int np4, double pt_jet);
