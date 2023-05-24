#pragma once

#include "addef.h"
#include <zlib.h>
#include <stddef.h>

// external classes
class Jet;
class GenParticle;
class Track;
class Tower;
class ADPDGQuerier;
class TLorentzVector;

// classes to be defined
class ADParticle;
class ADJet;

class ADParticle {

public:
  char feature_begin[0];
  Feature log_pt;
  Feature log_e;
  Feature log_pt_rel;
  Feature log_e_rel;
  Feature delta_r;
  Feature charge;
  Feature is_charged_hadron;
  Feature is_neutral_hadron;
  Feature is_photon;
  Feature is_electron;
  Feature is_muon;
  Feature d0;
  Feature d0_err;
  Feature dz;
  Feature dz_err;
  Feature deta;
  Feature dphi;
  Feature px;
  Feature py;
  Feature pz;
  Feature e;
  Feature mask;
  char feature_end[0];

  ADParticle();
  ADParticle(const GenParticle &, const TLorentzVector &p4_jet, const ADPDGQuerier &);
  ADParticle(const Track &, const TLorentzVector &p4_jet, const ADPDGQuerier &);
  ADParticle(const Tower &, const TLorentzVector &p4_jet, const ADPDGQuerier &);

  bool read(gzFile);  // binary input
  void write(gzFile) const;  // binary output

};


class ADJet {

public:
  char feature_begin[0];
  Feature pt;
  Feature eta;
  Feature phi;
  Feature e;
  Feature npar;
  Feature sdmass;
  Feature tau1;
  Feature tau2;
  Feature tau3;
  Feature tau4;
  char feature_end[0];

  ADParticle *par;

  ADJet(const ADPDGQuerier &, const Jet &);
  ~ADJet();

  bool read(gzFile);  // binary input
  void write(gzFile) const;  // binary output

  static size_t nadjet;
  static size_t nvalid;
  static size_t ngnpar;
  static size_t ntrack;
  static size_t ntower;
  static size_t nunrec;
  static size_t ndscrd;
  static void summary();  // particle counting summary

  static bool check(const Jet &);  // keep(true) or cut(false)

};

struct ADInvalidJet { };
