/*
 * Match gen-particles to jets.
 *
 * Author: <lyazj@github.com>
 * Date: Jun 21 2023
 */
#include "adgnmch.h"
#include <classes/DelphesClasses.h>
#include <TClonesArray.h>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <stdint.h>
#include <iostream>
#include <iomanip>

using namespace std;

void ADGenMatcher::set_gnpars(TClonesArray *arr)
{
  lastcc.clear();
  daumap.clear();
  gnpars.clear();

  // Loop over entries.
  Int_t n = arr->GetEntries();
  for(Int_t i = 0; i < n; ++i) {
    add_gnpar(arr, i);
  }

  // Match decay graphs.
  for(const ADGnparSP &gnpar : gnpars) {
    for(const ADDGMchSP &dgm : dgms) {
      dgm->add_gnpar(gnpar);
    }
  }
}

ADGnparSP ADGenMatcher::add_gnpar(TClonesArray *arr, int id)
{
  // Safely cast obj to gnpar.
  TObject *obj = arr->At(id);
  if(obj->IsA() != GenParticle::Class()) {
    throw invalid_argument("unexpected type: "s + obj->ClassName());
  }
  GenParticle *gnpar = (GenParticle *)obj;

  // Reentry avoidance.
  {
    auto it = lastcc.find(gnpar);
    if(it != lastcc.end()) return it->second;
  }
  ADGnparSP cc = NULL;

  // Share daughter lists with same (D1, D2).
  shared_ptr<vector<ADGnparSP>> daus;
  Int_t d1 = gnpar->D1, d2 = gnpar->D2;
  int64_t key = ((int64_t)d1 << 32) | (d2 & 0xffffffff);
  {
    auto it = daumap.find(key);
    if(it != daumap.end()) daus = it->second;
  }
  if(!daus) {
    daus.reset(new vector<ADGnparSP>);
    if(d1 >= 0) for(Int_t d = d1; d <= d2; ++d) {
      ADGnparSP dau = add_gnpar(arr, d);
      if(dau) {
        daus->push_back(dau);
        if(dau->pid == gnpar->PID) {
          if(cc == NULL || cc->p4.Pt() < dau->p4.Pt()) cc = dau;
        }
      }
    }
    daumap.emplace(key, daus);
  } else {
    for(const ADGnparSP &dau : *daus) {
      if(dau->pid == gnpar->PID) {
        if(cc == NULL || cc->p4.Pt() < dau->p4.Pt()) cc = dau;
      }
    }
  }

  // Add the particle conditionally.
  if(cc == NULL && pidsel.test(gnpar->PID)) {
    cc.reset(new ADGnpar{id, gnpar->PID, 0, gnpar->P4(), daus});
    gnpars.push_back(cc);
  }

  lastcc.emplace(gnpar, cc);
  return cc;
}

void ADGenMatcher::print_gnpars() const
{
  for(const ADGnparSP &gnpar : gnpars) {
    cout << setw(8) << gnpar->id << setw(8) << gnpar->pid;
    for(const ADGnparSP &gndau : *gnpar->daus) {
      cout << setw(8) << gndau->id;
    }
    cout << endl;
  };
}

ADPIDSelector P_T   (unordered_set<int>{6, -6});
ADPIDSelector P_W   (unordered_set<int>{24, -24});
ADPIDSelector P_C   (unordered_set<int>{4, -4});
ADPIDSelector P_Q   (unordered_set<int>{1, 2, 3, 4, -1, -2, -3, -4});
ADPIDSelector P_B   (unordered_set<int>{5, -5});
ADPIDSelector P_E   (unordered_set<int>{11, -11});
ADPIDSelector P_NE  (unordered_set<int>{12});
ADPIDSelector P_M   (unordered_set<int>{13, -13});
ADPIDSelector P_NM  (unordered_set<int>{14});
ADPIDSelector P_H   (unordered_set<int>{25});
ADPIDSelector P_G   (unordered_set<int>{21});
ADPIDSelector P_Z   (unordered_set<int>{23});
ADPIDSelector P_L   (unordered_set<int>{11, -11, 13, -13});
ADPIDSelector P_V   (unordered_set<int>{23, 24, -24});
ADPIDSelector P_QCD (unordered_set<int>{1, 2, 3, 4, 5, 6, -1, -2, -3, -4, -5, -6, 21});

void ADDecayGraph::add_dau(const ADDecayGSP &dau) {
  assert(!dau->par);
  dau->par = this;
  pidsel_all.merge(dau->pidsel_all);
  daus.push_back(dau);
}

ADDecayGraph::ADDecayGraph(const ADDecayGraph &other) {
  pidsel = other.pidsel;
  pidsel_all = other.pidsel_all;
  for(const ADDecayGSP &dau : other.daus) add_dau(std::make_shared<ADDecayGraph>(*dau));
}

bool ADDecayGraph::set_gnpar(const ADGnparSP &gnpar_in)
{
  if(pidsel.test(gnpar_in->pid)) {
    gnpar = gnpar_in;
    return true;
  }
  return false;
}

const ADDecayGraph G_T_BCQ { P_T, {
  make_shared<ADDecayGraph>(P_B),
  make_shared<ADDecayGraph>(P_W, vector<ADDecayGSP> {
    make_shared<ADDecayGraph>(P_C),
    make_shared<ADDecayGraph>(P_Q),
  }),
}};

const ADDecayGraph G_T_BQQ { P_T, {
  make_shared<ADDecayGraph>(P_B),
  make_shared<ADDecayGraph>(P_W, vector<ADDecayGSP> {
    make_shared<ADDecayGraph>(P_Q),
    make_shared<ADDecayGraph>(P_Q),
  }),
}};

const ADDecayGraph G_T_BC { P_T, {
  make_shared<ADDecayGraph>(P_B),
  make_shared<ADDecayGraph>(P_W, vector<ADDecayGSP> {
    make_shared<ADDecayGraph>(P_C),
  }),
}};

const ADDecayGraph G_T_BQ { P_T, {
  make_shared<ADDecayGraph>(P_B),
  make_shared<ADDecayGraph>(P_W, vector<ADDecayGSP> {
    make_shared<ADDecayGraph>(P_Q),
  }),
}};

const ADDecayGraph G_T_BEN { P_T, {
  make_shared<ADDecayGraph>(P_B),
  make_shared<ADDecayGraph>(P_W, vector<ADDecayGSP> {
    make_shared<ADDecayGraph>(P_E),
    make_shared<ADDecayGraph>(P_NE),
  }),
}};

const ADDecayGraph G_T_BMN { P_T, {
  make_shared<ADDecayGraph>(P_B),
  make_shared<ADDecayGraph>(P_W, vector<ADDecayGSP> {
    make_shared<ADDecayGraph>(P_M),
    make_shared<ADDecayGraph>(P_NM),
  }),
}};

const ADDecayGraph G_W_CQ { P_W, {
  make_shared<ADDecayGraph>(P_C),
  make_shared<ADDecayGraph>(P_Q),
}};

const ADDecayGraph G_W_QQ { P_W, {
  make_shared<ADDecayGraph>(P_Q),
  make_shared<ADDecayGraph>(P_Q),
}};

const ADDecayGraph G_Z_BB { P_Z, {
  make_shared<ADDecayGraph>(P_B),
  make_shared<ADDecayGraph>(P_B),
}};

const ADDecayGraph G_Z_CC { P_Z, {
  make_shared<ADDecayGraph>(P_C),
  make_shared<ADDecayGraph>(P_C),
}};

const ADDecayGraph G_Z_QQ { P_Z, {
  make_shared<ADDecayGraph>(P_Q),
  make_shared<ADDecayGraph>(P_Q),
}};

const ADDecayGraph G_H_BB { P_H, {
  make_shared<ADDecayGraph>(P_B),
  make_shared<ADDecayGraph>(P_B),
}};

const ADDecayGraph G_H_CC { P_H, {
  make_shared<ADDecayGraph>(P_C),
  make_shared<ADDecayGraph>(P_C),
}};

const ADDecayGraph G_H_QQ { P_H, {
  make_shared<ADDecayGraph>(P_Q),
  make_shared<ADDecayGraph>(P_Q),
}};

const ADDecayGraph G_H_GG { P_H, {
  make_shared<ADDecayGraph>(P_G),
  make_shared<ADDecayGraph>(P_G),
}};

const ADDecayGraph G_H_QQQQ { P_H, {
  make_shared<ADDecayGraph>(P_V, vector<ADDecayGSP> {
    make_shared<ADDecayGraph>(P_Q),
    make_shared<ADDecayGraph>(P_Q),
  }),
  make_shared<ADDecayGraph>(P_V, vector<ADDecayGSP> {
    make_shared<ADDecayGraph>(P_Q),
    make_shared<ADDecayGraph>(P_Q),
  }),
}};

const ADDecayGraph G_H_QQL { P_H, {
  make_shared<ADDecayGraph>(P_V, vector<ADDecayGSP> {
    make_shared<ADDecayGraph>(P_Q),
    make_shared<ADDecayGraph>(P_Q),
  }),
  make_shared<ADDecayGraph>(P_V, vector<ADDecayGSP> {
    make_shared<ADDecayGraph>(P_L),
  }),
}};

const ADDecayGraph G_QCD { P_QCD, {
  make_shared<ADDecayGraph>(P_QCD),
}};

void ADDGMatcher::add_gnpar(const ADGnparSP &gnpar)
{
  if(graph->set_gnpar(gnpar)) add_gnpar(1);
}

void ADDGMatcher::add_gnpar(size_t i)
{
  if(i == serial.size()) return record_entry();
  ADDecayGraph *g = serial[i];
  for(const ADGnparSP &dau : *g->get_par()->get_gnpar()->daus) {
    if(dau->tag || !g->set_gnpar(dau)) continue;
    dau->tag = 1;
    add_gnpar(i + 1);
    dau->tag = 0;
  }
}

void ADDGMatcher::record_entry()
{
  vector<ADGnparSP> entry;
  entry.reserve(serial.size());
  for(const ADDecayGraph *g : serial) {
    entry.emplace_back(g->get_gnpar());
  }
  gnpars.emplace_back(move(entry));
}

void ADDGMatcher::print_ngnpar() const
{
  cout << name << ": " << gnpars.size() << endl;
}

void ADDGMatcher::print_gnpars() const
{
  cout << name << ":" << endl;
  for(const vector<ADGnparSP> &entry : gnpars) {
    for(const ADGnparSP &gnpar : entry) {
      cout << " " << gnpar->id << "(" << gnpar->pid << ")";
    }
    cout << endl;
  };
}

void ADDecayGraph::serialize(std::vector<ADDecayGraph *> &s)
{
  s.push_back(this);
  for(const ADDecayGSP &dau : daus) {
    dau->serialize(s);
  }
}

ADGenMatcher::ADGenMatcher()
{
  // highest priority
  add_dgm(make_shared<ADDGMatcher>("T_BCQ",  make_shared<ADDecayGraph>(G_T_BCQ)));
  add_dgm(make_shared<ADDGMatcher>("T_BQQ",  make_shared<ADDecayGraph>(G_T_BQQ)));
  add_dgm(make_shared<ADDGMatcher>("T_BC",   make_shared<ADDecayGraph>(G_T_BC)));
  add_dgm(make_shared<ADDGMatcher>("T_BQ",   make_shared<ADDecayGraph>(G_T_BQ)));
  add_dgm(make_shared<ADDGMatcher>("T_BEN",  make_shared<ADDecayGraph>(G_T_BEN)));
  add_dgm(make_shared<ADDGMatcher>("T_BMN",  make_shared<ADDecayGraph>(G_T_BMN)));
  add_dgm(make_shared<ADDGMatcher>("H_QQQQ", make_shared<ADDecayGraph>(G_H_QQQQ)));
  add_dgm(make_shared<ADDGMatcher>("H_QQL",  make_shared<ADDecayGraph>(G_H_QQL)));
  add_dgm(make_shared<ADDGMatcher>("H_BB",   make_shared<ADDecayGraph>(G_H_BB)));
  add_dgm(make_shared<ADDGMatcher>("H_CC",   make_shared<ADDecayGraph>(G_H_CC)));
  add_dgm(make_shared<ADDGMatcher>("H_QQ",   make_shared<ADDecayGraph>(G_H_QQ)));
  add_dgm(make_shared<ADDGMatcher>("H_GG",   make_shared<ADDecayGraph>(G_H_GG)));
  add_dgm(make_shared<ADDGMatcher>("W_CQ",   make_shared<ADDecayGraph>(G_W_CQ)));
  add_dgm(make_shared<ADDGMatcher>("W_QQ",   make_shared<ADDecayGraph>(G_W_QQ)));
  add_dgm(make_shared<ADDGMatcher>("Z_BB",   make_shared<ADDecayGraph>(G_Z_BB)));
  add_dgm(make_shared<ADDGMatcher>("Z_CC",   make_shared<ADDecayGraph>(G_Z_CC)));
  add_dgm(make_shared<ADDGMatcher>("Z_QQ",   make_shared<ADDecayGraph>(G_Z_QQ)));
  add_dgm(make_shared<ADDGMatcher>("QCD",    make_shared<ADDecayGraph>(G_QCD)));
  // lowest priority
}

void ADGenMatcher::print_dgms() const
{
  for(const ADDGMchSP &dgm : dgms) {
    dgm->print_ngnpar();
  }
}

ADGenMatchResult ADDGMatcher::match(Jet *jet, double dr)
{
  TLorentzVector p4 = jet->P4();
  const vector<ADGnparSP> *matching = NULL;
  double dr_sum_min = INFINITY;

  for(const auto &entry : gnpars) {
    bool matched = true;
    double dr_sum = 0.0;
    for(const ADGnparSP &gnpar : entry) {
      double dr_cur = p4.DeltaR(gnpar->p4);
      if(dr_cur > dr) {
        matched = false;
        break;
      }
      dr_sum += dr_cur;
    }
    if(matched && dr_sum < dr_sum_min) {
      dr_sum_min = dr_sum;
      matching = &entry;
    }
  }

  if(!matching) return { NULL, { }, INFINITY };
  return { name, *matching, dr_sum_min / (1 + serial.size()) };
}

ADGenMatchResult ADGenMatcher::match(Jet *jet, double dr)
{
  for(const ADDGMchSP &dgm : dgms) {
    ADGenMatchResult rst = dgm->match(jet, dr);
    if(rst.name) return rst;
  }
  return { NULL, { }, INFINITY };
}
