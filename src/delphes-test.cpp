#include "adenv.h"
#include <classes/DelphesClasses.h>
#include <TFile.h>
#include <TTree.h>
#include <memory>
#include <stdio.h>

using namespace std;

int main(int argc, char *argv[])
{
  const char *rootfile;  // rootfile to be handled
  if(argc != 2) {
    fprintf(stderr, "usage: %s <rootfile>\n", get_invoc_short_name());
    return 1;
  }
  rootfile = argv[1];

  // NOTE: Do this before loading rootfile.
  setenv_delphes();

  // Link libdelphes.so.
  Jet();

  // Read rootfile and print the tree.
  auto file = make_unique<TFile>(rootfile, "read");
  auto delphes = (TTree *)file->Get("Delphes");
  delphes->Print();

  return 0;
}
