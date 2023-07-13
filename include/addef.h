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

#ifndef JET_BRANCH
#define JET_BRANCH  "FatJet"_branch
#endif  /* JET_BRANCH */

#ifndef ECF_N2_NP4
#define ECF_N2_NP4  128
#endif  /* ECF_N2_NP4 */

#ifndef ECF_N2_BETA
#define ECF_N2_BETA  1.0
#endif  /* ECF_N2_BETA */

#ifndef ECF_N3_NP4
#define ECF_N3_NP4  32
#endif  /* ECF_N3_NP4 */

#ifndef ECF_N3_BETA
#define ECF_N3_BETA  2.0
#endif  /* ECF_N3_BETA */

struct ADException {
  virtual ~ADException() = default;
};

extern const char *class_names[NRSLTCLASS];
