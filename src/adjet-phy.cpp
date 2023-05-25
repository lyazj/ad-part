#include "addef.h"
#include "adjet.h"
#include "adpdg.h"
#include <classes/DelphesClasses.h>
#include <TObject.h>
#include <TLorentzVector.h>
#include <new>
#include <math.h>
#include <stdio.h>

using namespace std;

size_t ADJet::nadjet;
size_t ADJet::nvalid;
size_t ADJet::ngnpar;
size_t ADJet::ntrack;
size_t ADJet::ntower;
size_t ADJet::nunrec;
size_t ADJet::ndscrd;

namespace {

template<class DelphesClass>
bool check_par_common(const TObject &par)
{
  TLorentzVector p4 = ((const DelphesClass &)par).P4();
  return abs(p4.Pz()) <= 10000 && abs(p4.Eta()) <= 5 && p4.Pt() > 0;
}

template<class DelphesClass>
void set_adpar_common(ADParticle &adpar, const DelphesClass &par,
    const TLorentzVector &p4_jet, ADPDGInfo pdginfo)
{
  TLorentzVector p4 = par.P4();

  adpar.log_pt = log(p4.Pt());
  adpar.log_e = log(p4.Energy());
  adpar.log_pt_rel = adpar.log_pt - log(p4_jet.Pt());
  adpar.log_e_rel = adpar.log_e - log(p4_jet.Energy());
  adpar.delta_r = p4.DeltaR(p4_jet);
  adpar.charge = 0.0;  // padding
  adpar.is_charged_hadron = pdginfo.is_charged_hadron();
  adpar.is_neutral_hadron = pdginfo.is_neutral_hadron();
  adpar.is_photon = pdginfo.is_photon();
  adpar.is_electron = pdginfo.is_electron();
  adpar.is_muon = pdginfo.is_muon();
  adpar.d0 = 0.0;  // padding
  adpar.d0_err = 0.0;  // padding
  adpar.dz = 0.0; // padding
  adpar.dz_err = 0.0;  // padding
  adpar.deta = (p4.Eta() > 0 ? : -1) * (p4.Eta() - p4_jet.Eta());
  adpar.dphi = p4.DeltaPhi(p4_jet);
  adpar.px = p4.Px();
  adpar.py = p4.Py();
  adpar.pz = p4.Pz();
  adpar.e = p4.Energy();
  adpar.mask = 1.0;
}

template<class DelphesClass>
void set_adpar_charge(ADParticle &adpar, const DelphesClass &par,
    const TLorentzVector &, ADPDGInfo)
{
  adpar.charge = par.Charge;
}

template<class DelphesClass>
void set_adpar_d0dzde(ADParticle &adpar, const DelphesClass &par,
    const TLorentzVector &, ADPDGInfo)
{
  adpar.d0 = par.D0;
  adpar.d0_err = par.ErrorD0;
  adpar.dz = par.DZ;
  adpar.dz_err = par.ErrorDZ;
}

// max_{i: size_t}{(size_t)(float)i == i}
constexpr auto NPAR_MAX = 0x1000000;

}  // namespace

ADParticle::ADParticle(const GenParticle &gnpar,
    const TLorentzVector &p4_jet, const ADPDGQuerier &pdg)
{
  ADPDGInfo pdginfo = pdg[gnpar.PID];
  set_adpar_common(*this, gnpar, p4_jet, pdginfo);
  set_adpar_charge(*this, gnpar, p4_jet, pdginfo);
}

ADParticle::ADParticle(const Track &track,
    const TLorentzVector &p4_jet, const ADPDGQuerier &pdg)
{
  ADPDGInfo pdginfo = pdg[track.PID];
  set_adpar_common(*this, track, p4_jet, pdginfo);
  set_adpar_charge(*this, track, p4_jet, pdginfo);
  set_adpar_d0dzde(*this, track, p4_jet, pdginfo);
}

ADParticle::ADParticle(const Tower &tower,
    const TLorentzVector &p4_jet, const ADPDGQuerier &)
{
  set_adpar_common(*this, tower, p4_jet, (ADPDGInfo)0);
}

ADJet::ADJet(const ADPDGQuerier &pdg, const Jet &jet) : ADJet()
{
  ++nadjet;
  if(!check(jet)) throw ADInvalidJet();
  ++nvalid;

  TLorentzVector p4 = jet.P4();
  Long64_t n = jet.Constituents.GetEntries();

  // features
  pt = p4.Pt();
  eta = p4.Eta();
  phi = p4.Phi();
  e = p4.Energy();
  npar = n;
  sdmass = jet.SoftDroppedP4[0].M();
  tau1 = jet.Tau[0];
  tau2 = jet.Tau[1];
  tau3 = jet.Tau[2];
  tau4 = jet.Tau[3];

  // constituents
  Long64_t c = 0;  // particle counter
  for(Long64_t i = 0; i < n && c < NPARTIFLOW; ++i) {
    TObject *obj = jet.Constituents[i];
    if(obj->IsA() == GenParticle::Class()) {
      ++ngnpar;
      if(!check_par_common<GenParticle>(*obj)) { ++ndscrd; continue; }
      new((void *)&par[c++]) ADParticle(*(GenParticle *)obj, p4, pdg);
    }
    else if(obj->IsA() == Track::Class()) {
      ++ntrack;
      if(!check_par_common<Track>(*obj)) { ++ndscrd; continue; }
      new((void *)&par[c++]) ADParticle(*(Track *)obj, p4, pdg);
    } else if(obj->IsA() == Tower::Class()) {
      ++ntower;
      if(!check_par_common<Tower>(*obj)) { ++ndscrd; continue; }
      new((void *)&par[c++]) ADParticle(*(Tower *)obj, p4, pdg);
    } else {  // unrecognized type
      ++nunrec;
      fprintf(stderr, "WARNING: unrecognized constituent type\n");
    }
  }

  // sorting
  // TODO better performance
  auto larger_pt = [](const ADParticle &p, const ADParticle &q) {
    return p.log_pt > q.log_pt;
  };
  sort(par, par + c, larger_pt);

  // feature updating
  if(c > NPAR_MAX) throw ADOverflow();
  npar = c;

  // padding
  while(c < NPARTIFLOW) {
    new((void *)&par[c++]) ADParticle();
  }
}

void ADJet::summary()
{
  printf("nadjet=%zu nvalid=%zu ngnpar=%zu ntrack=%zu ntower=%zu nunrec=%zu "
      "ndscrd=%zu\n", nadjet, nvalid, ngnpar, ntrack, ntower, nunrec, ndscrd);
}

bool ADJet::check(const Jet &jet)  // XXX
{
  // return jet.PT >= 500 && abs(jet.Eta) <= 2;
  return abs(jet.Eta) <= 2;
}
