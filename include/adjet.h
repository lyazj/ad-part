#pragma once

#include "addef.h"
#include <zlib.h>
#include <stddef.h>

// external classes
class Jet;
class GenParticle;
class Track;
class Tower;
class Electron;
class Muon;
class ADPDGQuerier;
class TLorentzVector;
class ParticleFlowCandidate;
class ScalarHT;
class MissingET;

// classes to be defined
class ADParticle;
class ADJet;

struct ADInvalidJet : ADException { };
struct ADOverflow : ADException { };

class ADParticle {

public:
  char feature_begin[0];

  Feature pf_features_begin[0];
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
  Feature pf_points_begin[0];
  Feature deta;
  Feature dphi;
  Feature pf_points_end[0];
  Feature pf_features_end[0];

  Feature pf_vectors_begin[0];
  Feature px;
  Feature py;
  Feature pz;
  Feature e;
  Feature pf_vectors_end[0];

  Feature pf_mask_begin[0];
  Feature mask;
  Feature pf_mask_end[0];

  char feature_end[0];

  ADParticle();
  ADParticle(const GenParticle &, const TLorentzVector &p4_jet, const ADPDGQuerier &);
  ADParticle(const Track &, const TLorentzVector &p4_jet, const ADPDGQuerier &);
  ADParticle(const Tower &, const TLorentzVector &p4_jet, const ADPDGQuerier &);
  ADParticle(const Muon &, const TLorentzVector &p4_jet, const ADPDGQuerier &);
  ADParticle(const ParticleFlowCandidate &, const TLorentzVector &p4_jet, const ADPDGQuerier &);

  bool read(gzFile);  // binary input
  void write(gzFile) const;  // binary output

private:
  void preprocess();

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

  char ecf_begin[0];
  Feature n2_0;
  Feature n3_0;
  Feature n3_1;
  Feature n3_2;
  Feature n3_3;
  Feature n3_4;
  Feature n3_5;
  Feature n3_6;
  Feature n3_7;
  Feature n3_8;
  Feature n3_9;
  Feature n3_10;
  char ecf_end[0];

  Feature tau21;
  Feature tau32;
  Feature tau43;
  Feature ftrec;
  Feature label;

  char feature_end[0];

  ADParticle *par;

  ADJet();
  ADJet(const ADPDGQuerier &, const Jet &, const char *name);
  ~ADJet();
  static bool check(const Jet &);  // keep(true) or cut(false)

  bool read(gzFile);  // binary input
  void write(gzFile) const;  // binary output

  static size_t nadjet;
  static size_t nvalid;
  static size_t ngnpar;
  static size_t ntrack;
  static size_t ntower;
  static size_t ncmuon;
  static size_t nparfc;
  static size_t nunrec;
  static size_t ndscrd;
  static size_t nunmch;
  static size_t nirrlv;
  static size_t nclass[NRSLTCLASS];
  static void summary();  // particle counting summary

};

class ADParT {

public:
  char feature_begin[0];
  Feature part[NRSLTCLASS];
  char feature_end[0];

  bool read(gzFile);  // binary input
  void write(gzFile) const;  // binary output

};

class ADLepton {

public:
  char feature_begin[0];
  Feature pt;
  Feature eta;
  Feature phi;
  Feature e;
  Feature charge;
  Feature iso_db;
  Feature iso_rc;
  Feature d0;
  Feature d0_err;
  Feature dz;
  Feature dz_err;
  char feature_end[0];

  ADLepton();
  ADLepton(const Electron &);
  ADLepton(const Muon &);

  bool read(gzFile);  // binary input
  void write(gzFile) const;  // binary output

};

class ADEvent {

public:
  char feature_begin[0];
  Feature ht;
  Feature met;
  Feature metphi;
  Feature njet;
  Feature nlep;
  Feature label;
  char feature_end[0];

  void set_ht(const ScalarHT &);
  void set_met(const MissingET &);

  bool read(gzFile);  // binary input
  void write(gzFile) const;  // binary output

};
