#include "adenv.h"
#include "adjet.h"
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <err.h>

using namespace std;

int main(int argc, char *argv[])
{
  if(argc == 1) {
    fprintf(stderr, "usage: %s <jetfile> [ <jetfile> ... ]\n", get_invoc_short_name());
    return 1;
  }

  for(int i = 1; i < argc; ++i) {
    const char *jetpath = argv[i];
    gzFile jetfile = gzopen(jetpath, "r");
    if(jetfile == NULL) {
      errx(EXIT_FAILURE, "failed to open jet file: %s", jetpath);
    }

    ADJet jet;
    while(jet.read(jetfile)) {
      fwrite(jet.feature_begin, jet.feature_end - jet.feature_begin, 1, stdout);
    }
  }

  return 0;
}
