#pragma once

#include "addef.h"
#include <zlib.h>
#include <stddef.h>

// external classes
class Jet;
class GenParticle;
class Track;
class Tower;
class Muon;
class ADPDGQuerier;
class TLorentzVector;
class ParticleFlowCandidate;

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
