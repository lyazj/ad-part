#include "adenv.h"
#include "adjet.h"
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <err.h>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{
  if(argc <= 2) {
    fprintf(stderr, "usage: %s <label> <evtfile> [ <evtfile> ... ]\n", get_invoc_short_name());
    return 1;
  }
  int label = stoi(argv[1]);

  for(int i = 2; i < argc; ++i) {
    const char *evtpath = argv[i];
    gzFile evtfile = gzopen(evtpath, "r");
    if(evtfile == NULL) {
      errx(EXIT_FAILURE, "failed to open evt file: %s", evtpath);
    }

    ADEvent evt;
    while(evt.read(evtfile)) {
      if(label >= 0) evt.label = label;
      fwrite(evt.feature_begin, evt.feature_end - evt.feature_begin, 1, stdout);
    }
  }

  return 0;
}
