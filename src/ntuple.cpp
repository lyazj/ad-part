#include "addef.h"
#include "adenv.h"
#include "adjet.h"
#include <TFile.h>
#include <TTree.h>
#include <zlib.h>
#include <stdio.h>
#include <vector>
#include <memory>

using namespace std;

int main(int argc, char *argv[])
{
  // Force line-buffered output streams.
  setlbf();

  if(argc != 3) {
    fprintf(stderr, "usage: %s <dumpfile> <rootfile>\n",
        get_invoc_short_name());
    return 1;
  }
  const char *dumpfile = argv[1];  // dumpfile to read
  const char *rootfile = argv[2];  // rootfile to write

  // Open dumpfile to read.
  gzFile dump = gzopen(dumpfile, "rb");
  if(dump == NULL) {
    fprintf(stderr, "ERROR: error opening dumpfile\n");
    return 1;
  }
  shared_ptr<gzFile_s> dump_guard(dump, [](gzFile f) { gzclose(f); } );

  // Open rootfile to write.
  auto root = make_shared<TFile>(rootfile, "recreate");
  if(!root->IsOpen()) {
    fprintf(stderr, "ERROR: error opening rootfile\n");
    return 1;
  }

  // In-memory data structure.
  vector<pair<const char *, Feature>> sdata = {
    {"label_QCD",  0},
    {"label_Hbb",  0},
    {"label_Hcc",  0},
    {"label_Hgg",  0},
    {"label_H4q",  0},
    {"label_Hqql", 0},
    {"label_Zqq",  0},
    {"label_Wqq",  0},
    {"label_Tbqq", 0},
    {"label_Tbl",  0},
  };
  vector<pair<const char *, vector<Feature>>> vdata = {
    {"part_pt_log",          { }},
    {"part_e_log",           { }},
    {"part_logptrel",        { }},
    {"part_logerel",         { }},
    {"part_deltaR",          { }},
    {"part_charge",          { }},
    {"part_isChargedHadron", { }},
    {"part_isNeutralHadron", { }},
    {"part_isPhoton",        { }},
    {"part_isElectron",      { }},
    {"part_isMuon",          { }},
    {"part_d0",              { }},
    {"part_d0err",           { }},
    {"part_dz",              { }},
    {"part_dzerr",           { }},
    {"part_deta",            { }},
    {"part_dphi",            { }},
    {"part_px",              { }},
    {"part_py",              { }},
    {"part_pz",              { }},
    {"part_energy",          { }},
    {"part_mask",            { }},
  };

  // Create ROOT TTree with desired branches.
  auto tree = make_shared<TTree>("tree", "tree");
  for(auto &[name, buf] : sdata) {
    tree->Branch(name, &buf);
  }
  for(auto &[name, buf] : vdata) {
    tree->Branch(name, &buf);
    buf.reserve(NPARTIFLOW);
  }

  // Copy data.
  size_t n = 0;
  ADJet jet;
  while(jet.read(dump)) {
    int label = jet.label;
    assert(label >= 0 && label < NRSLTCLASS);
    for(auto &[name, buf] : sdata) {
      buf = 0.0;
    }
    sdata[label].second = 1.0;
    for(auto &[name, buf] : vdata) {
      buf.clear();
    }
    size_t npar = jet.npar;
    for(size_t i = 0; i < npar; ++i) {
      Feature *feature = (Feature *)jet.par[i].feature_begin;
      size_t j = 0;
      for(auto &[name, buf] : vdata) {
        buf.push_back(feature[j++]);
      }
    }
    tree->Fill();
    if((++n % 1000) == 0) {
      printf("%s: %zu entries processed\n", dumpfile, n);
    }
  }
  tree->Write();

  // Finish.
  printf("%s: %zu entries total\n", dumpfile, n);
  return 0;
}
