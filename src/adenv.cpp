#include "adenv.h"
#include <string>
#include <errno.h>
#include <unistd.h>

using namespace std;

const char *get_invoc_short_name()
{
  return program_invocation_short_name;
}

void setenv_delphes()
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
