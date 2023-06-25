#include "addef.h"
#include "adenv.h"
#include "adpdg.h"
#include "adtype.h"
#include "adbranch.h"
#include "adjet.h"
#include <classes/DelphesClasses.h>
#include <TFile.h>
#include <TTree.h>
#include <memory>
#include <zlib.h>
#include <stdio.h>

using namespace std;

int main(int argc, char *argv[])
{
  // Force line-buffered output streams.
  setlbf();

  if(argc < 3) {
    fprintf(stderr, "usage: %s <rootfile> [ <rootfile> ... ] <dumpfile>\n",
        get_invoc_short_name());
    return 1;
  }

  // Open dumpfile to write.
  const char *dumpfile = argv[argc - 1];  // dumpfile to write
  gzFile dump = gzopen(dumpfile, "wb");
  if(dump == NULL) {
    fprintf(stderr, "ERROR: error opening dumpfile\n");
    return 1;
  }
  shared_ptr<gzFile_s> dump_guard(dump, [](gzFile f) { gzclose(f); } );

  ADPDGQuerier pdg;  // pdgid -> type info

  // NOTE: Do this before loading rootfile.
  setenv_delphes();

  // Traverse input rootfiles.
  for(int a = 1; a < argc - 1; ++a) {
    // Open rootfile and get Delphes tree.
    const char *rootfile = argv[a];  // rootfile to read
    auto file = make_shared<TFile>(rootfile, "read");
    auto delphes = (TTree *)file->Get("Delphes");
    Long64_t n = delphes->GetEntries();
    printf("%s: %llu events total\n", rootfile, (unsigned long long)n);

    // Set up branches.
    auto brjet = get_branch(delphes, JET_BRANCH);

    // Traverse entries.
    for(Long64_t i = 0; i < n; ++i) {
      // Fetch data.
      delphes->GetEntry(i);

      // Parse and dump data.
      size_t njet = brjet.size();
      for(size_t j = 0; j < njet; ++j) {
        try {
          ADJet jet(pdg, *brjet[j], "QCD");
          jet.write(dump);
        } catch(const ADInvalidJet &) { }
      }
      if((i + 1) % 1000 == 0) {
        printf("%s: %llu events processed\n", rootfile, (unsigned long long)(i + 1));
      }
    }
  }

  // Success!
  ADJet::summary();
  return 0;
}
