#pragma once

#ifndef Feature
#define Feature  float
#endif  /* Feature */

#ifndef PDG_PATH
#define PDG_PATH  "pdginfo"
#endif  /* PDG_PATH */

#ifndef BATCH_SIZE
#define BATCH_SIZE  128
#endif  /* BATCH_SIZE */

#ifndef NPARTIFLOW
#define NPARTIFLOW  128
#endif  /* NPARTIFLOW */

#ifndef NRSLTCLASS
#define NRSLTCLASS  10
#endif  /* NRSLTCLASS */

#ifndef NJETPEVENT
#define NJETPEVENT  10
#endif  /* NJETPEVENT */

struct ADException {
  virtual ~ADException() = default;
};

extern const char *class_names[NRSLTCLASS];
