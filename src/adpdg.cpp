#include "adpdg.h"
#include <unordered_map>
#include <stdexcept>
#include <stdio.h>
#include <memory>

using namespace std;

class ADPDGQuerier::Internal {

public:
  unordered_map<int, int> data;

};

ADPDGQuerier::ADPDGQuerier(const char *pdgfile)
{
  // C file options give the clearest result.
  FILE *file = fopen(pdgfile, "rb");
  if(file == NULL) {
    throw runtime_error("error opening file: "s + pdgfile);
  }

  // The guard ensures that the file be closed before leaving the scope.
  auto file_guard = shared_ptr<FILE>(file, [](FILE *f) { fclose(f); });

  // Load data from pdgfile.
  int buf[2];
  unordered_map<int, int> data;
  for(;;) {
    size_t r = fread(buf, 1, sizeof buf, file);
    if(r != sizeof buf) {
      if(r == 0 && feof(file)) break;
      throw runtime_error("error reading file: "s + pdgfile);
    }
    data.emplace(buf[0], buf[1]);
  }

  /*
   * <lyazj@github.com> added on Jun 26 2023
   * This is a strange setting but does matter a lot.
   */
  data.emplace(buf[0], PT_NEUTRAL_HADRON);

  // If this fails, move construction must not be performed.
  // No memory leak is possible.
  internal = new Internal { .data = move(data) };
}

ADPDGQuerier::~ADPDGQuerier()
{
  delete internal;
}

ADPDGInfo ADPDGQuerier::operator[](int pdgid) const
{
  int info = 0;  // means not a known class
  auto iter = internal->data.find(pdgid);
  if(iter != internal->data.end()) {
    info = iter->second;
  }
  return ADPDGInfo(info);
}
