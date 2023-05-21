#include "addef.h"
#include "adjet.h"
#include "adpdg.h"
#include <classes/DelphesClasses.h>
#include <TObject.h>
#include <TLorentzVector.h>
#include <stdexcept>
#include <algorithm>
#include <new>
#include <zlib.h>
#include <math.h>
#include <stdio.h>

using namespace std;

size_t ADJet::ngnpar;
size_t ADJet::ntrack;
size_t ADJet::ntower;
size_t ADJet::nunrec;

namespace {

template<class T>
void write_features(const T &obj, gzFile file)
{
  size_t s = obj.feature_end - obj.feature_begin;
  size_t r = gzfwrite(obj.feature_begin, 1, s, file);
  if(r != s) {
    int e;
    throw runtime_error(gzerror(file, &e));
  }
}

template<class T>
bool read_features(T &obj, gzFile file)
{
  size_t s = obj.feature_end - obj.feature_begin;
  size_t r = gzfread(obj.feature_begin, 1, s, file);
  if(r != s) {
    int e;
    const char *msg = gzerror(file, &e);
    if(e || r) {
      throw runtime_error(msg);
    }
    return false;
  }
  return true;
}

}  // namespace

ADParticle::ADParticle()
{
  memset(feature_begin, 0, feature_end - feature_begin);  // padding
}

ADParticle::ADParticle(const ADPDGQuerier &pdg, const GenParticle &gnpar, const ADJet &jet)
{
  TLorentzVector p4 = gnpar.P4();
  ADPDGInfo pdginfo = pdg[gnpar.PID];

  log_pt = log(p4.Pt());
  log_e = log(p4.Energy());
  log_pt_rel = log_pt - log(jet.pt);
  log_e_rel = log_e - log(jet.e);
  delta_r = hypot(p4.Eta() - jet.eta, p4.Phi() - jet.phi);
  charge = gnpar.Charge;
  is_charged_hadron = pdginfo.is_charged_hadron();
  is_neutral_hadron = pdginfo.is_neutral_hadron();
  is_photon = pdginfo.is_photon();
  is_electron = pdginfo.is_electron();
  is_muon = pdginfo.is_muon();
  d0 = 0.0;  // padding
  d0_err = 0.0;  // padding
  dz = 0.0; // padding
  dz_err = 0.0;  // padding
  deta = p4.Eta() - jet.eta;
  dphi = p4.Phi() - jet.phi;
  px = p4.Px();
  py = p4.Py();
  pz = p4.Pz();
  e = p4.Energy();
  mask = 1.0;
}

ADParticle::ADParticle(const ADPDGQuerier &pdg, const Track &track, const ADJet &jet)
{
  TLorentzVector p4 = track.P4();
  ADPDGInfo pdginfo = pdg[track.PID];

  log_pt = log(p4.Pt());
  log_e = log(p4.Energy());
  log_pt_rel = log_pt - log(jet.pt);
  log_e_rel = log_e - log(jet.e);
  delta_r = hypot(p4.Eta() - jet.eta, p4.Phi() - jet.phi);
  charge = track.Charge;
  is_charged_hadron = pdginfo.is_charged_hadron();
  is_neutral_hadron = pdginfo.is_neutral_hadron();
  is_photon = pdginfo.is_photon();
  is_electron = pdginfo.is_electron();
  is_muon = pdginfo.is_muon();
  d0 = track.D0;
  d0_err = track.ErrorD0;
  dz = track.DZ;
  dz_err = track.ErrorDZ;
  deta = p4.Eta() - jet.eta;
  dphi = p4.Phi() - jet.phi;
  px = p4.Px();
  py = p4.Py();
  pz = p4.Pz();
  e = p4.Energy();
  mask = 1.0;
}

ADParticle::ADParticle(const ADPDGQuerier &, const Tower &tower, const ADJet &jet)
{
  TLorentzVector p4 = tower.P4();
  ADPDGInfo pdginfo(0);  // padding

  log_pt = log(p4.Pt());
  log_e = log(p4.Energy());
  log_pt_rel = log_pt - log(jet.pt);
  log_e_rel = log_e - log(jet.e);
  delta_r = hypot(p4.Eta() - jet.eta, p4.Phi() - jet.phi);
  charge = 0.0;  // padding
  is_charged_hadron = pdginfo.is_charged_hadron();
  is_neutral_hadron = pdginfo.is_neutral_hadron();
  is_photon = pdginfo.is_photon();
  is_electron = pdginfo.is_electron();
  is_muon = pdginfo.is_muon();
  d0 = 0.0;  // padding
  d0_err = 0.0;  // padding
  dz = 0.0; // padding
  dz_err = 0.0;  // padding
  deta = p4.Eta() - jet.eta;
  dphi = p4.Phi() - jet.phi;
  px = p4.Px();
  py = p4.Py();
  pz = p4.Pz();
  e = p4.Energy();
  mask = 1.0;
}

bool ADParticle::read(gzFile file)
{
  return read_features(*this, file);
}

void ADParticle::write(gzFile file) const
{
  write_features(*this, file);
}

ADJet::ADJet(const ADPDGQuerier &pdg, const Jet &jet)
{
  TLorentzVector p4 = jet.P4();
  Long64_t n = jet.Constituents.GetEntries();

  pt = p4.Pt();
  eta = p4.Eta();
  phi = p4.Phi();
  e = p4.Energy();
  npar = n;
  sdmass = jet.Mass;  // ?
  tau1 = jet.Tau[0];  // ?
  tau2 = jet.Tau[1];  // ?
  tau3 = jet.Tau[2];  // ?
  tau4 = jet.Tau[3];  // ?

  n = min<Long64_t>(n, NPAR_PER_JET);
  par = (ADParticle *)malloc(NPAR_PER_JET * sizeof(*par));
  if(par == NULL) throw bad_alloc();

  for(Long64_t i = 0; i < n; ++i) {
    TObject *obj = jet.Constituents[i];
    if(obj->IsA() == GenParticle::Class()) {
      ++ngnpar;
      new((void *)&par[i]) ADParticle(pdg, *(GenParticle *)obj, *this);
    }
    else if(obj->IsA() == Track::Class()) {
      ++ntrack;
      new((void *)&par[i]) ADParticle(pdg, *(Track *)obj, *this);
    } else if(obj->IsA() == Tower::Class()) {
      ++ntower;
      new((void *)&par[i]) ADParticle(pdg, *(Tower *)obj, *this);
    } else {  // unrecognized type
      ++nunrec;
      fprintf(stderr, "WARNING: unrecognized constituent type\n");
      new((void *)&par[n]) ADParticle();
    }
  }
  for(; n < NPAR_PER_JET; ++n) {
    new((void *)&par[n]) ADParticle();
  }
}

ADJet::~ADJet()
{
  for(Long64_t i = 0; i < NPAR_PER_JET; ++i) {
    par[i].~ADParticle();
  }
  free(par);
}

bool ADJet::read(gzFile file)
{
  if(!read_features(*this, file)) return false;
  for(Long64_t i = 0; i < NPAR_PER_JET; ++i) {
    if(!par[i].read(file)) {
      throw runtime_error("unexpected EOF");
    }
  }
  return true;
}

void ADJet::write(gzFile file) const
{
  write_features(*this, file);
  for(Long64_t i = 0; i < NPAR_PER_JET; ++i) {
    par[i].write(file);
  }
}

void ADJet::summary()
{
  printf("ngnpar=%zu ntrack=%zu ntower=%zu nunrec=%zu\n",
      ngnpar, ntrack, ntower, nunrec);
}
