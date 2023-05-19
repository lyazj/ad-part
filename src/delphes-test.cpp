#include <classes/DelphesClasses.h>
#include <TFile.h>
#include <TTree.h>
#include <memory>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;

static void append_delphes_incpath()
{
  const char *cpinc = getenv("CPLUS_INCLUDE_PATH");
  string incpath = cpinc ? : "";

#ifdef DELPHES_DIR
  incpath += (incpath.empty() ? ""s : ":"s) + DELPHES_DIR;
#endif  /* DELPHES_DIR */

#ifdef DELPHES_EXTERNAL_DIR
  incpath += (incpath.empty() ? ""s : ":"s) + DELPHES_EXTERNAL_DIR;
#endif  /* DELPHES_EXTERNAL_DIR */

  if(!incpath.empty()) setenv("CPLUS_INCLUDE_PATH", incpath.c_str(), 1);
}

int main(int argc, char *argv[])
{
  const char *rootfile;  // rootfile to be handle
  if(argc != 2) {
    fprintf(stderr,
        "usage: %s <rootfile>\n", program_invocation_short_name);
    return 1;
  }
  rootfile = argv[1];

  // NOTE: do this before loading rootfile
  append_delphes_incpath();

  // read rootfile and print the tree
  auto file = make_shared<TFile>(rootfile, "read");
  auto delphes = file->Get<TTree>("Delphes");
  delphes->Print();

  return 0;
}
