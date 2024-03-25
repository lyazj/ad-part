#include "addef.h"
#include "adenv.h"
#include "adjet.h"
#include <TFile.h>
#include <TTree.h>
#include <zlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <memory>

using namespace std;

int main(int argc, char *argv[])
{
  // Force line-buffered output streams.
  setlbf();

  if(argc != 8) {
    fprintf(stderr, "usage: %s <evtfile> <jetfile> <lepfile> <phofile>"
        " <outfile> <hidfile> <rootfile>\n", get_invoc_short_name());
    return 1;
  }
  const char *evtpath = argv[1];  // evtfile to read
  const char *jetpath = argv[2];  // jetfile to read
  const char *leppath = argv[3];  // lepfile to read
  const char *phopath = argv[4];  // phofile to read
  const char *outpath = argv[5];  // outfile to read
  const char *hidpath = argv[6];  // hidfile to read
  const char *rootpath = argv[7];  // rootfile to write

  // Open input files to read.
  gzFile evtfile = gzopen(evtpath, "rb");
    if(evtfile == NULL) { fprintf(stderr, "ERROR: error opening evtfile\n"); return 1; }
    shared_ptr<gzFile_s> evtguard(evtfile, [](gzFile f) { gzclose(f); } );
  gzFile jetfile = gzopen(jetpath, "rb");
    if(jetfile == NULL) { fprintf(stderr, "ERROR: error opening jetfile\n"); return 1; }
    shared_ptr<gzFile_s> jetguard(jetfile, [](gzFile f) { gzclose(f); } );
  gzFile lepfile = gzopen(leppath, "rb");
    if(lepfile == NULL) { fprintf(stderr, "ERROR: error opening lepfile\n"); return 1; }
    shared_ptr<gzFile_s> lepguard(lepfile, [](gzFile f) { gzclose(f); } );
  gzFile phofile = gzopen(phopath, "rb");
    if(phofile == NULL) { fprintf(stderr, "ERROR: error opening phofile\n"); return 1; }
    shared_ptr<gzFile_s> phoguard(phofile, [](gzFile f) { gzclose(f); } );
  gzFile outfile = gzopen(outpath, "rb");
    if(outfile == NULL) { fprintf(stderr, "ERROR: error opening outfile\n"); return 1; }
    shared_ptr<gzFile_s> outguard(outfile, [](gzFile f) { gzclose(f); } );
  gzFile hidfile = gzopen(hidpath, "rb");
    if(hidfile == NULL) { fprintf(stderr, "ERROR: error opening hidfile\n"); return 1; }
    shared_ptr<gzFile_s> hidguard(hidfile, [](gzFile f) { gzclose(f); } );

  // Open rootfile to write.
  auto rootfile = make_shared<TFile>(rootpath, "recreate");
  if(!rootfile->IsOpen()) { fprintf(stderr, "ERROR: error opening rootfile\n"); return 1; }

  // In-memory data structure.
  vector<pair<const char *, vector<Feature>>> evtvec = {
    {"evt_ht",          { }},
    {"evt_met",         { }},
    {"evt_metphi",      { }},
    {"evt_njet",        { }},
    {"evt_nlep",        { }},
    {"evt_npho",        { }},
    {"evt_label",       { }},
  };
  vector<pair<const char *, vector<Feature>>> jetvec = {
    {"jet_pt",          { }},
    {"jet_eta",         { }},
    {"jet_phi",         { }},
    {"jet_e",           { }},
    {"jet_npar",        { }},
    {"jet_sdmass",      { }},
    {"jet_tau1",        { }},
    {"jet_tau2",        { }},
    {"jet_tau3",        { }},
    {"jet_tau4",        { }},
    {"jet_n2_0",        { }},
    {"jet_n3_0",        { }},
    {"jet_n3_1",        { }},
    {"jet_n3_2",        { }},
    {"jet_n3_3",        { }},
    {"jet_n3_4",        { }},
    {"jet_n3_5",        { }},
    {"jet_n3_6",        { }},
    {"jet_n3_7",        { }},
    {"jet_n3_8",        { }},
    {"jet_n3_9",        { }},
    {"jet_n3_10",       { }},
    {"jet_tau21",       { }},
    {"jet_tau32",       { }},
    {"jet_tau43",       { }},
    {"jet_ftrec",       { }},
    {"jet_label",       { }},
  };
  vector<pair<const char *, vector<Feature>>> lepvec = {
    {"lep_pt",          { }},
    {"lep_eta",         { }},
    {"lep_phi",         { }},
    {"lep_e",           { }},
    {"lep_charge",      { }},
    {"lep_iso_db",      { }},
    {"lep_iso_rc",      { }},
    {"lep_d0",          { }},
    {"lep_d0_err",      { }},
    {"lep_dz",          { }},
    {"lep_dz_err",      { }},
  };
  vector<pair<const char *, vector<Feature>>> phovec = {
    {"pho_pt",          { }},
    {"pho_eta",         { }},
    {"pho_phi",         { }},
    {"pho_e",           { }},
    {"pho_t",           { }},
    {"pho_ehoe",        { }},
    {"pho_iso_db",      { }},
    {"pho_iso_rc",      { }},
    {"pho_sumpt_c",     { }},
    {"pho_sumpt_n",     { }},
    {"pho_sumpt_c_pu",  { }},
    {"pho_sumpt",       { }},
    {"pho_status",      { }},
  };
  vector<pair<string, vector<Feature>>> outvec;
  outvec.reserve(NRSLTCLASS);
  for(size_t i = 0; i < NRSLTCLASS; ++i) {
    outvec.push_back({"out_" + to_string(i), { }});
  }
  vector<pair<string, vector<Feature>>> hidvec;
  hidvec.reserve(NHIDDEN);
  for(size_t i = 0; i < NHIDDEN; ++i) {
    hidvec.push_back({"hid_" + to_string(i), { }});
  }

  // Create ROOT TTree with desired branches.
  auto tree = make_shared<TTree>("tree", "tree");
  for(auto &[name, buf] : evtvec) { tree->Branch(name, &buf); buf.resize(1); }
  for(auto &[name, buf] : jetvec) { tree->Branch(name, &buf); buf.reserve(10); }
  for(auto &[name, buf] : lepvec) { tree->Branch(name, &buf); buf.reserve(10); }
  for(auto &[name, buf] : phovec) { tree->Branch(name, &buf); buf.reserve(10); }
  for(auto &[name, buf] : outvec) { tree->Branch(name.c_str(), &buf); buf.reserve(10); }
  for(auto &[name, buf] : hidvec) { tree->Branch(name.c_str(), &buf); buf.reserve(10); }

  // Copy data.
  size_t n = 0;
  ADEvent evt;
  ADJet jet;
  ADLepton lep;
  ADPhoton pho;
  ADParTOutput out;
  ADParTHidden hid;
  while(evt.read(evtfile)) {

    size_t njet, nlep, npho;

    // Event features.
    {
      Feature *feature = (Feature *)evt.feature_begin;
      for(auto it = evtvec.begin(); it != evtvec.end(); ++it) {
        it->second[0] = *feature++;
      }
      njet = evtvec[3].second[0];
      nlep = evtvec[4].second[0];
      npho = evtvec[5].second[0];
    }

    // Jet features.
    for(auto it = jetvec.begin(); it != jetvec.end(); ++it) it->second.clear();
    for(auto it = outvec.begin(); it != outvec.end(); ++it) it->second.clear();
    for(auto it = hidvec.begin(); it != hidvec.end(); ++it) it->second.clear();
    for(size_t ijet = 0; ijet < njet; ++ijet) {
      if(!(jet.read_without_particles(jetfile))) { fprintf(stderr, "ERROR: error reading jetfile\n"); return 1; }
      Feature *feature = (Feature *)jet.feature_begin;
      for(auto it = jetvec.begin(); it != jetvec.end(); ++it) {
        it->second.push_back(*feature++);
      }

      if(!(out.read(outfile))) { fprintf(stderr, "ERROR: error reading outfile\n"); return 1; }
      feature = (Feature *)out.feature_begin;
      for(auto it = outvec.begin(); it != outvec.end(); ++it) {
        it->second.push_back(*feature++);
      }

      if(!(hid.read(hidfile))) { fprintf(stderr, "ERROR: error reading hidfile\n"); return 1; }
      feature = (Feature *)hid.feature_begin;
      for(auto it = hidvec.begin(); it != hidvec.end(); ++it) {
        it->second.push_back(*feature++);
      }
    }

    // Lepton features.
    for(auto it = lepvec.begin(); it != lepvec.end(); ++it) it->second.clear();
    for(size_t ilep = 0; ilep < nlep; ++ilep) {
      if(!(lep.read(lepfile))) { fprintf(stderr, "ERROR: error reading lepfile\n"); return 1; }
      Feature *feature = (Feature *)lep.feature_begin;
      for(auto it = lepvec.begin(); it != lepvec.end(); ++it) {
        it->second.push_back(*feature++);
      }
    }

    // Photon features.
    for(auto it = phovec.begin(); it != phovec.end(); ++it) it->second.clear();
    for(size_t ipho = 0; ipho < npho; ++ipho) {
      if(!(pho.read(phofile))) { fprintf(stderr, "ERROR: error reading phofile\n"); return 1; }
      Feature *feature = (Feature *)pho.feature_begin;
      for(auto it = phovec.begin(); it != phovec.end(); ++it) {
        it->second.push_back(*feature++);
      }
    }

    tree->Fill();
    if((++n % 1000) == 0) {
      printf("%s: %zu entries processed\n", rootpath, n);
    }
  }
  tree->Write();

  // Finish.
  printf("%s: %zu entries total\n", rootpath, n);
  return 0;
}
