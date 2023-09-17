#include "addef.h"
#include "adenv.h"
#include "adpdg.h"
#include "adtype.h"
#include "adbranch.h"
#include "adjet.h"
#include "adgnmch.h"
#include <classes/DelphesClasses.h>
#include <TFile.h>
#include <TTree.h>
#include <memory>
#include <zlib.h>
#include <stdio.h>

using namespace std;

#ifndef JET_BRANCH
#define JET_BRANCH  "FatJet"_branch
#endif  /* JET_BRANCH */

#ifndef GNMCH_DETR
#define GNMCH_DETR  0.8
#endif  /* GNMCH_DETR */

register_branch("Particle"_pack, "GenParticle"_type);
register_branch("MissingET"_pack, "MissingET"_type);

int main(int argc, char *argv[])
{
  // Force line-buffered output streams.
  setlbf();

  if(argc < 5) {
    fprintf(stderr, "usage: %s <label> <jetfile> <evtfile> <rootfile> [ <rootfile> ... ]\n",
        get_invoc_short_name());
    return 1;
  }

  // Get event label.
  Feature label = stoi(argv[1]);

  // Open jetfile to write.
  gzFile jetfile = gzopen(argv[2], "wb");
  if(jetfile == NULL) {
    fprintf(stderr, "ERROR: error opening jetfile\n");
    return 1;
  }
  shared_ptr<gzFile_s> jetfile_guard(jetfile, [](gzFile f) { gzclose(f); } );

  // Open evtfile to write.
  gzFile evtfile = gzopen(argv[3], "wb");
  if(evtfile == NULL) {
    fprintf(stderr, "ERROR: error opening evtfile\n");
    return 1;
  }
  shared_ptr<gzFile_s> evtfile_guard(evtfile, [](gzFile f) { gzclose(f); } );

  ADPDGQuerier pdg;  // pdgid -> type info

  // NOTE: Do this before loading rootfile.
  setenv_delphes();

  ////ADGenMatcher matcher;

  // Traverse input rootfiles.
  for(int a = 4; a < argc; ++a) {
    // Open rootfile and get Delphes tree.
    const char *rootfile = argv[a];  // rootfile to read
    auto file = make_shared<TFile>(rootfile, "read");
    if(!file->IsOpen()) {
      fprintf(stderr, "ERROR: error opening rootfile\n");
      continue;
    }
    auto delphes = (TTree *)file->Get("Delphes");
    Long64_t n = delphes->GetEntries();
    printf("%s: %llu events total\n", rootfile, (unsigned long long)n);

    // Set up branches.
    auto brjet = get_branch(delphes, JET_BRANCH);
    auto brgpar = get_branch(delphes, "Particle"_branch);
    //auto brmet = get_branch(delphes, "MissingET"_branch);

    // Traverse entries.
    for(Long64_t i = 0; i < n; ++i) {
      // Fetch data.
      delphes->GetEntry(i);

      // Parse and dump data.
      ////matcher.set_gnpars(brgpar.get_data());
      //matcher.print_gnpars();
      //matcher.print_dgms();
      //printf("\n");
      size_t njet = brjet.size();
      if(njet == 0) continue;
      for(size_t j = 0; j < njet; ++j) {
        ////ADGenMatchResult r = matcher.match(brjet[j], GNMCH_DETR);
        //if(!r.name) {
        //  printf("result: null\n");
        //} else {
        //  printf("result: %-8s%10.6lf", r.name, r.dr_mean);
        //  for(const ADGnparSP &gnpar : r.gnpars) {
        //    printf("%6d(%4d)", gnpar->id, gnpar->pid);
        //  }
        //  printf("\n");
        //}
        //printf("\n");
        try {
          ADJet jet(pdg, *brjet[j], "QCD");  // NOTE: "QCD" is only a placeholder.
          jet.write(jetfile);
        } catch(const ADInvalidJet &) { }
      }
      ADEvent evt;
      //evt.set_met(*brmet[0]);
      evt.met = 0.0;
      evt.metphi = 0.0;
      evt.njet = njet;
      evt.label = label;
      evt.write(evtfile);
      if((i + 1) % 100 == 0) {
        ADJet::summary();
        printf("%s: %llu events processed\n", rootfile, (unsigned long long)(i + 1));
      }
    }
  }

  // Success!
  ADJet::summary();
  return 0;
}
