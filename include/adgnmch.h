/*
 * Match gen-particles to jets.
 *
 * Author: <lyazj@github.com>
 * Date: Jun 21 2023
 */
#pragma once

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <TLorentzVector.h>

// External classes.
class TClonesArray;
class GenParticle;
class Jet;

struct ADGnpar;
typedef std::shared_ptr<ADGnpar> ADGnparSP;

struct ADGnpar {
  int id;
  int pid;
  int tag;
  TLorentzVector p4;
  std::shared_ptr<std::vector<ADGnparSP>> daus;
};

class ADPIDSelector {
private:
  std::unordered_set<int> expected;

public:
  ADPIDSelector(const std::unordered_set<int> &s = { }) : expected(s) { }
  bool test(int pid) const { return expected.count(pid); }
  void merge(const ADPIDSelector &other) { expected.insert(other.expected.begin(), other.expected.end()); }
};

extern ADPIDSelector P_T, P_W, P_C, P_Q, P_B, P_E, P_NE, P_M, P_NM, P_H, P_G, P_Z, P_L, P_V, P_QCD;

class ADDecayGraph;
typedef class std::shared_ptr<ADDecayGraph> ADDecayGSP;

class ADDecayGraph {
private:
  ADPIDSelector pidsel;
  ADPIDSelector pidsel_all;
  std::vector<ADDecayGSP> daus;
  ADDecayGraph *par = NULL;
  ADGnparSP gnpar;

  void serialize(std::vector<ADDecayGraph *> &s);

public:
  ADDecayGraph(const ADPIDSelector &ps = { }, const std::vector<ADDecayGSP> &daus_in = { })
    : pidsel(ps), pidsel_all(ps) { for(const auto &dau : daus_in) add_dau(dau); }
  void add_dau(const ADDecayGSP &dau);
  ADDecayGraph(const ADDecayGraph &other);
  ~ADDecayGraph() { for(ADDecayGSP &dau : daus) dau->par = NULL; }
  std::vector<ADDecayGraph *> serialize() { std::vector<ADDecayGraph *> s; serialize(s); return s; }
  ADDecayGraph *get_par() const { return par; }
  bool set_gnpar(const ADGnparSP &gnpar_in);
  const ADGnparSP &get_gnpar() const { return gnpar; }
  const ADPIDSelector &get_pidsel_all() const { return pidsel_all; }
};

extern const ADDecayGraph G_T_BCQ, G_T_BQQ, G_T_BC, G_T_BQ, G_T_BEN, G_T_BMN;
extern const ADDecayGraph G_W_CQ, G_W_QQ;
extern const ADDecayGraph G_Z_BB, G_Z_CC, G_Z_QQ;
extern const ADDecayGraph G_H_BB, G_H_CC, G_H_QQ, G_H_GG, G_H_QQQQ, G_H_QQL;
extern const ADDecayGraph G_QCD;

struct ADGenMatchResult {
  const char *name;
  std::vector<ADGnparSP> gnpars;
  double dr_mean;
};

class ADDGMatcher {
private:
  const char *name;
  ADDecayGSP graph;
  std::vector<ADDecayGraph *> serial;
  std::vector<std::vector<ADGnparSP>> gnpars;

  void add_gnpar(size_t);
  void record_entry();

public:
  ADDGMatcher(const char *n, const ADDecayGSP &g) : name(n), graph(g), serial(g->serialize()) { }
  void add_gnpar(const ADGnparSP &gnpar);
  const ADPIDSelector &get_pidsel_all() const { return graph->get_pidsel_all(); }
  void print_gnpars() const;
  void print_ngnpar() const;
  ADGenMatchResult match(Jet *, double);
};

typedef std::shared_ptr<ADDGMatcher> ADDGMchSP;

class ADGenMatcher {
private:
  std::vector<ADDGMchSP> dgms;
  ADPIDSelector pidsel;
  std::unordered_map<GenParticle *, ADGnparSP> lastcc;
  std::unordered_map<int64_t, std::shared_ptr<std::vector<ADGnparSP>>> daumap;
  std::vector<ADGnparSP> gnpars;

  ADGnparSP add_gnpar(TClonesArray *, int);  // returns the last C.C. on interest, NULL otherwise
  void add_dgm(const ADDGMchSP &dgm) { dgms.push_back(dgm); pidsel.merge(dgm->get_pidsel_all()); }

public:
  ADGenMatcher();

  void set_gnpars(TClonesArray *);
  void print_gnpars() const;
  void print_dgms() const;
  ADGenMatchResult match(Jet *, double);
};
