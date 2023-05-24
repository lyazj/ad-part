#pragma once

#ifndef Feature
#define Feature  float
#endif  /* Feature */

#ifndef NPAR_PER_JET
#define NPAR_PER_JET  128
#endif  /* NPAR_PER_JET */

#ifndef PDG_PATH
#define PDG_PATH  "pdginfo"
#endif  /* PDG_PATH */

struct ADException {
  virtual ~ADException() = default;
};
