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

ADPIDSelector P_T   { 6, -6 };
ADPIDSelector P_W   { 24, -24 };
ADPIDSelector P_C   { 4, -4 };
ADPIDSelector P_Q   { 1, 2, 3, 4, -1, -2, -3, -4 };
ADPIDSelector P_B   { 5, -5 };
ADPIDSelector P_E   { 11, -11 };
ADPIDSelector P_NE  { 12 };
ADPIDSelector P_M   { 13, -13 };
ADPIDSelector P_NM  { 14 };
ADPIDSelector P_H   { 25 };
ADPIDSelector P_G   { 21 };
ADPIDSelector P_Z   { 23 };
ADPIDSelector P_L   { 11, -11, 13, -13 };
ADPIDSelector P_V   { 23, 24, -24 };
ADPIDSelector P_QCD { 1, 2, 3, 4, 5, 6, -1, -2, -3, -4, -5, -6, 21 };

void ADDecayGraph::add_dau(const ADDecayGSP &dau) {
  dau->par = this;
  pidsel_union.merge(dau->pidsel_union);
  daus.emplace_back(dau);
}

void ADDecayGraph::add_dau(ADDecayGSP &&dau) {
  dau->par = this;
  pidsel_union.merge(dau->pidsel_union);
  daus.emplace_back(move(dau));  // No significant difference.
}

bool ADDecayGraph::set_gnpar(const ADGnparSP &gnpar_in)
{
  if(pidsel.test(gnpar_in->pid)) {
    gnpar = gnpar_in;
    return true;
  }
  return false;  // Nothing changed.
}

void ADDecayGraph::serialize(std::vector<ADDecayGraph *> &s)
{
  s.push_back(this);
  for(const ADDecayGSP &dau : daus) {
    dau->serialize(s);
  }
}

const ADDecayGraph G_T_BCQ { P_T, {
  new ADDecayGraph { P_B },
  new ADDecayGraph { P_W, { new ADDecayGraph(P_C), new ADDecayGraph(P_Q) } },
} };

const ADDecayGraph G_T_BQQ { P_T, {
  new ADDecayGraph { P_B },
  new ADDecayGraph { P_W, { new ADDecayGraph(P_Q), new ADDecayGraph(P_Q) } },
} };

const ADDecayGraph G_T_BC { P_T, {
  new ADDecayGraph { P_B },
  new ADDecayGraph { P_W, { new ADDecayGraph(P_C) } },
} };

const ADDecayGraph G_T_BQ { P_T, {
  new ADDecayGraph { P_B },
  new ADDecayGraph { P_W, { new ADDecayGraph(P_Q) } },
} };

const ADDecayGraph G_T_BEN { P_T, {
  new ADDecayGraph { P_B },
  new ADDecayGraph { P_W, { new ADDecayGraph(P_E), new ADDecayGraph(P_NE) } },
} };

const ADDecayGraph G_T_BMN { P_T, {
  new ADDecayGraph { P_B },
  new ADDecayGraph { P_W, { new ADDecayGraph(P_M), new ADDecayGraph(P_NM) } },
} };

const ADDecayGraph G_W_CQ { P_W, { new ADDecayGraph(P_C), new ADDecayGraph(P_Q) } };
const ADDecayGraph G_W_QQ { P_W, { new ADDecayGraph(P_Q), new ADDecayGraph(P_Q) } };

const ADDecayGraph G_Z_BB { P_Z, { new ADDecayGraph(P_B), new ADDecayGraph(P_B) } };
const ADDecayGraph G_Z_CC { P_Z, { new ADDecayGraph(P_C), new ADDecayGraph(P_C) } };
const ADDecayGraph G_Z_QQ { P_Z, { new ADDecayGraph(P_Q), new ADDecayGraph(P_Q) } };

const ADDecayGraph G_H_BB { P_H, { new ADDecayGraph(P_B), new ADDecayGraph(P_B) } };
const ADDecayGraph G_H_CC { P_H, { new ADDecayGraph(P_C), new ADDecayGraph(P_C) } };
const ADDecayGraph G_H_QQ { P_H, { new ADDecayGraph(P_Q), new ADDecayGraph(P_Q) } };
const ADDecayGraph G_H_GG { P_H, { new ADDecayGraph(P_G), new ADDecayGraph(P_G) } };

const ADDecayGraph G_H_QQQQ { P_H, {
  new ADDecayGraph { P_V, { new ADDecayGraph(P_Q), new ADDecayGraph(P_Q) } },
  new ADDecayGraph { P_V, { new ADDecayGraph(P_Q), new ADDecayGraph(P_Q) } },
} };

const ADDecayGraph G_H_QQL { P_H, {
  new ADDecayGraph { P_V, { new ADDecayGraph(P_Q), new ADDecayGraph(P_Q) } },
  new ADDecayGraph { P_V, { new ADDecayGraph(P_L) } },
} };

const ADDecayGraph G_QCD { P_QCD, { new ADDecayGraph(P_QCD) } };  // XXX

void ADDGMatcher::add_gnpar(const ADGnparSP &gnpar)
{
  if(graph->set_gnpar(gnpar)) add_gnpar(1);  // Root matching goes first.
}

void ADDGMatcher::add_gnpar(size_t i)  // Then enumerate all possible cases.
{
  if(i == serial.size()) return record_entry();  // Success.
  ADDecayGraph *g = serial[i];
  // Trail object of parent node must be valid as matching is performed in pre-order.
  for(const ADGnparSP &dau : *g->get_par()->get_gnpar()->daus) {
    if(dau->tag || !g->set_gnpar(dau)) continue;  // Duplicate matching is avoided with TAG.
    dau->tag = 1;  // Means already used for trail matching.
    add_gnpar(i + 1);  // Move on for a deeper recursion.
    dau->tag = 0;  // Backtrace.
  }
}

void ADDGMatcher::record_entry()  // XXX: The records are permutation-sensitive.
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

void ADGenMatcher::set_gnpars(TClonesArray *arr)
{
  lastcc.clear();
  daumap.clear();
  gnpars.clear();

  // Loop over entries.
  Int_t n = arr->GetEntries();
  for(Int_t i = 0; i < n; ++i) {
    add_gnpar(arr, i);  // Irrelevant ones are discarded.
  }

  // Match decay graphs.
  for(const ADGnparSP &gnpar : gnpars) {
    for(const ADDGMtchrSP &dgm : dgms) {
      dgm->add_gnpar(gnpar);
    }
  }
}

ADGnparSP ADGenMatcher::add_gnpar(TClonesArray *arr, int id)
{
  // Safely cast OBJ to GenParticle *.
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
      ADGnparSP dau = add_gnpar(arr, d);  // DFS
      if(dau) {  // Should be discarded otherwise.
        daus->push_back(dau);
        if(dau->pid == gnpar->PID) {  // Higher-pT C.C. is preferred.
          if(cc == NULL || cc->p4.Pt() < dau->p4.Pt()) cc = dau;
        }
      }
    }
    daumap.emplace(key, daus);
  } else {  // Perform C.C. matching only.
    for(const ADGnparSP &dau : *daus) {
      if(dau->pid == gnpar->PID) {
        if(cc == NULL || cc->p4.Pt() < dau->p4.Pt()) cc = dau;
      }
    }
  }

  // Add the particle conditionally.
  //   CC not NULL: intermediate state -> discard
  //   PID test failed: out of domain -> discard
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

ADGenMatcher::ADGenMatcher()
{
  // highest priority
  add_dgm("T_BCQ", G_T_BCQ);
  add_dgm("T_BQQ", G_T_BQQ);
  add_dgm("T_BC", G_T_BC);
  add_dgm("T_BQ", G_T_BQ);
  add_dgm("T_BEN", G_T_BEN);
  add_dgm("T_BMN", G_T_BMN);
  add_dgm("H_QQQQ", G_H_QQQQ);
  add_dgm("H_QQL", G_H_QQL);
  add_dgm("H_BB", G_H_BB);
  add_dgm("H_CC", G_H_CC);
  add_dgm("H_QQ", G_H_QQ);
  add_dgm("H_GG", G_H_GG);
  add_dgm("W_CQ", G_W_CQ);
  add_dgm("W_QQ", G_W_QQ);
  add_dgm("Z_BB", G_Z_BB);
  add_dgm("Z_CC", G_Z_CC);
  add_dgm("Z_QQ", G_Z_QQ);
  add_dgm("QCD", G_QCD);
  // lowest priority
}

void ADGenMatcher::print_dgms() const
{
  for(const ADDGMtchrSP &dgm : dgms) {
    dgm->print_ngnpar();
  }
}

ADGenMatchResult ADDGMatcher::match(Jet *jet, double dr)
{
  TLorentzVector p4 = jet->P4();
  const vector<ADGnparSP> *matching = NULL;
  double dr_sum_min = INFINITY;

  // The entry with minimal DR_SUM is used.
  for(const auto &entry : gnpars) {
    bool matched = true;
    double dr_sum = 0.0;
    for(const ADGnparSP &gnpar : entry) {
      double dr_cur = p4.DeltaR(gnpar->p4);
      if(dr_cur > dr) {  // DR is the common upper bound of Delta-R.
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
  // The first matching is used.
  for(const ADDGMtchrSP &dgm : dgms) {
    ADGenMatchResult rst = dgm->match(jet, dr);
    if(rst.name) return rst;
  }
  return { NULL, { }, INFINITY };
}
