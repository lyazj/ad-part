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
#include <initializer_list>

// External classes.
class TClonesArray;
class GenParticle;
class Jet;

// In-memory representation of GenParticle, irrelevant fields discarded.
struct ADGnpar;
typedef std::shared_ptr<ADGnpar> ADGnparSP;
struct ADGnpar {
  int id;  // For debugging.
  int pid;  // PDG identification.
  int tag;  // For recursive matching algorithm.
  TLorentzVector p4;  // For Delta-R computing.
  std::shared_ptr<std::vector<ADGnparSP>> daus;  // Mid-state discarded.
};

// Perform PID-determined GenParticle selection.
class ADPIDSelector {
private:
  std::unordered_set<int> expected;

public:
  // Construct the selector with set/list-like objects.
  ADPIDSelector() { }
  ADPIDSelector(const std::unordered_set<int> &s) : expected(s) { }
  ADPIDSelector(std::unordered_set<int> &&s) : expected(move(s)) { }
  ADPIDSelector(std::initializer_list<int> &&il) : expected(move(il)) { }

  // Test whether a PID matches expected criteria.
  bool test(int pid) const { return expected.count(pid); }

  // Do UNION operation on accepting domains.
  void merge(const ADPIDSelector &other) { expected.insert(other.expected.begin(), other.expected.end()); }
};

// Implement HEP notations of particles.
extern const ADPIDSelector P_T, P_W, P_C, P_Q, P_B, P_E, P_NE, P_M, P_NM, P_H, P_G, P_Z, P_L, P_V, P_QCD;

// Perform decay-determined GenParticle selection and classification.
class ADDecayGraph;
typedef class std::shared_ptr<ADDecayGraph> ADDecayGSP;
class ADDecayGraph {
private:
  ADPIDSelector pidsel;  // Selection criteria of this vertex.
  ADPIDSelector pidsel_union;  // Union of SCs of all sub-vertices.
  std::vector<ADDecayGSP> daus;  // Sub-vertices, defined recursively.
  ADDecayGraph *par = NULL;  // Parent node, NULL for root.
  ADGnparSP gnpar;  // Temporary slot for trail matching.

  void serialize(std::vector<ADDecayGraph *> &s);  // Get pre-order sequence (implementation).

public:
  // Decay Graphs are constructed hierarchically.
  ADDecayGraph(const ADPIDSelector &ps, const std::vector<ADDecayGSP> &daus_in)
    : pidsel(ps), pidsel_union(ps) { for(const ADDecayGSP &dau : daus_in) add_dau(dau); }
  ADDecayGraph(const ADPIDSelector &ps, std::vector<ADDecayGSP> &&daus_in)
    : pidsel(ps), pidsel_union(ps) { for(const ADDecayGSP &dau : daus_in) add_dau(move(dau)); }
  ADDecayGraph(const ADPIDSelector &ps, std::initializer_list<ADDecayGraph *> &&daus_in)
    : pidsel(ps), pidsel_union(ps) { for(ADDecayGraph *dau : daus_in) add_dau(ADDecayGSP(dau)); }
  ADDecayGraph(const ADPIDSelector &ps) : ADDecayGraph(ps, { }) { }
  ADDecayGraph() { }

  // Copy/move construction is value-oriented. GNPAR is not copied/moved.
  ADDecayGraph(const ADDecayGraph &other) : pidsel(other.pidsel), pidsel_union(other.pidsel_union)
    { for(const ADDecayGSP &dau : other.daus) add_dau(std::make_shared<ADDecayGraph>(*dau)); }
  ADDecayGraph(ADDecayGraph &&other) : pidsel(std::move(other.pidsel)), pidsel_union(std::move(other.pidsel_union))
    { for(const ADDecayGSP &dau : other.daus) add_dau(move(dau)); }

  // Detach each child node on destruction.
  ~ADDecayGraph() { for(const ADDecayGSP &dau : daus) dau->par = NULL; }

  // Attach child nodes.
  void add_dau(const ADDecayGSP &dau);
  void add_dau(ADDecayGSP &&dau);

  // User interface for pre-order sequence.
  std::vector<ADDecayGraph *> serialize() { std::vector<ADDecayGraph *> s; serialize(s); return s; }

  // SET operation succeeds if and only if criteria of this node is matched.
  bool set_gnpar(const ADGnparSP &gnpar_in);
  const ADGnparSP &get_gnpar() const { return gnpar; }

  // Read-only access to other data structures of this node.
  ADDecayGraph *get_par() const { return par; }
  const ADPIDSelector &get_pidsel_union() const { return pidsel_union; }
};

// Implement HEP notations of particle decays.
extern const ADDecayGraph G_T_BCQ, G_T_BQQ, G_T_BC, G_T_BQ, G_T_BEN, G_T_BMN;
extern const ADDecayGraph G_W_CQ, G_W_QQ;
extern const ADDecayGraph G_Z_BB, G_Z_CC, G_Z_QQ;
extern const ADDecayGraph G_H_BB, G_H_CC, G_H_QQ, G_H_GG, G_H_QQQQ, G_H_QQL;
extern const ADDecayGraph G_QCD;

// Valid only if name != NULL.
struct ADGenMatchResult {
  const char *name;
  std::vector<ADGnparSP> gnpars;
  double dr_mean;
};

// Match GenParticles to decay graphs, then match Jets to graphs.
class ADDGMatcher {
private:
  const char *name;  // Name of decay process.
  ADDecayGSP graph;
  std::vector<ADDecayGraph *> serial;  // Pre-order sequence of GRAPH.
  std::vector<std::vector<ADGnparSP>> gnpars;  // Candidate entries ordered as SERIAL.

  void add_gnpar(size_t);  // Inner matching recursion.
  void record_entry();  // Save a all-matched case.

public:
  ADDGMatcher(const char *n, const ADDecayGSP &g) : name(n), graph(g), serial(g->serialize()) { }

  // User interface for adding GenParticles for matching.
  void add_gnpar(const ADGnparSP &gnpar);
  void print_gnpars() const;  // Debugging use (detailed).
  void print_ngnpar() const;  // Debugging use (concise).

  // Match Jets to decay graphs with matching GenParticles.
  ADGenMatchResult match(Jet *, double);

  // Read-only access to other data structures of the graph.
  const ADPIDSelector &get_pidsel_union() const { return graph->get_pidsel_union(); }
};
typedef std::shared_ptr<ADDGMatcher> ADDGMtchrSP;

// The outer-most concealing of the particle-to-decay-process matching.
class ADGenMatcher {
private:
  std::vector<ADDGMtchrSP> dgms;  // In descending order of priority.
  ADPIDSelector pidsel;  // Union of accepting domains of DGMS.
  std::unordered_map<GenParticle *, ADGnparSP> lastcc;  // Inner DS for duplication elimination.
  std::unordered_map<int64_t, std::shared_ptr<std::vector<ADGnparSP>>> daumap;  // A possible time- and memory-saving mechanism.
  std::vector<ADGnparSP> gnpars;  // Filtered by PIDSEL.

  // Graphs should be added in descending order of priority.
  void add_dgm(const ADDGMtchrSP &dgm) { dgms.push_back(dgm); pidsel.merge(dgm->get_pidsel_union()); }
  void add_dgm(const char *nm, const ADDecayGraph &dg)
    { add_dgm(std::make_shared<ADDGMatcher>(nm, std::make_shared<ADDecayGraph>(dg))); }

  ADGnparSP add_gnpar(TClonesArray *, int);  // Returns the last C.C. if on interest, NULL otherwise.

public:
  ADGenMatcher();  // Graphs are added here.

  void set_gnpars(TClonesArray *);  // Each entry of ARRAY must points to GenParticle.
  ADGenMatchResult match(Jet *, double);  // Should be called after set_gnpars().

  // Diagnostic methods.
  void print_gnpars() const;
  void print_dgms() const;
};
