#pragma once

#ifndef Feature
#define Feature  float
#endif  /* Feature */

#ifndef PDG_PATH
#define PDG_PATH  "pdginfo"
#endif  /* PDG_PATH */

#ifndef BATCH_SIZE
#define BATCH_SIZE  1
#endif  /* BATCH_SIZE */

#ifndef NPARTIFLOW
#define NPARTIFLOW  128
#endif  /* NPARTIFLOW */

#ifndef NRSLTCLASS
#define NRSLTCLASS  188
#endif  /* NRSLTCLASS */

#ifndef NHIDDEN
#define NHIDDEN  128
#endif  /* NHIDDEN */

struct ADException {
  virtual ~ADException() = default;
};

extern const char *class_names[NRSLTCLASS];
