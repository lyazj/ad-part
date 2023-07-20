#include "addef.h"
#include "adenv.h"
#include "adjet.h"
#include <TFile.h>
#include <TTree.h>
#include <zlib.h>
#include <stdio.h>
#include <string.h>
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
    {"jet_pt",     0},
    {"jet_eta",    0},
    {"jet_phi",    0},
    {"jet_e",      0},
    {"jet_npar",   0},
    {"jet_sdmass", 0},
    {"jet_tau1",   0},
    {"jet_tau2",   0},
    {"jet_tau3",   0},
    {"jet_tau4",   0},
    {"jet_n2_0",   0},
    {"jet_n3_0",   0},
    {"jet_n3_1",   0},
    {"jet_n3_2",   0},
    {"jet_n3_3",   0},
    {"jet_n3_4",   0},
    {"jet_n3_5",   0},
    {"jet_n3_6",   0},
    {"jet_n3_7",   0},
    {"jet_n3_8",   0},
    {"jet_n3_9",   0},
    {"jet_n3_10",  0},
    {"jet_tau21",  0},
    {"jet_tau32",  0},
    {"jet_tau43",  0},
    {"jet_ftrec",  0},
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
  auto jdata_begin = sdata.begin();
  auto jdata_end = jdata_begin;
  while(jdata_end != sdata.end() && !strncmp(jdata_end->first, "jet_", 4)) {
    ++jdata_end;
  }
  auto ldata_begin = jdata_end;
  auto ldata_end = sdata.end();
  while(jet.read(dump)) {

    // Jet features.
    {
      Feature *feature = (Feature *)jet.feature_begin;
      for(auto it = jdata_begin; it != jdata_end; ++it) {
        it->second = *feature++;
      }
    }

    // Labels.
    int label = jet.label;
    assert(label >= 0 && label < NRSLTCLASS);
    for(auto it = ldata_begin; it != ldata_end; ++it) {
      it->second = distance(ldata_begin, it) == label;
    }

    // Particle features.
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
