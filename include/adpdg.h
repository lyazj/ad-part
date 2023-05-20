#pragma once

#include "addef.h"

#define PT_CHARGED_HADRON   1
#define PT_NEUTRAL_HADRON   2
#define PT_PHOTON           4
#define PT_ELECTRON         8
#define PT_MUON            16

class ADPDGInfo {

public:
  explicit ADPDGInfo(int data_in): data(data_in) { }

  bool is_charged_hadron() const { return data & PT_CHARGED_HADRON; }
  bool is_neutral_hadron() const { return data & PT_NEUTRAL_HADRON; }
  bool is_photon() const { return data & PT_PHOTON; }
  bool is_electron() const { return data & PT_ELECTRON; }
  bool is_muon() const { return data & PT_MUON; }

private:
  int data;

};

class ADPDGQuerier {

public:
  explicit ADPDGQuerier(const char *pdgfile);
  ADPDGQuerier() : ADPDGQuerier(PDG_PATH) { }
  ~ADPDGQuerier();

  ADPDGInfo operator[](int pdgid) const;

private:
  class Internal;
  class Internal *internal;

};
