#pragma once

#include "addef.h"
#include <stdint.h>

#ifndef BATCH_SIZE
#define BATCH_SIZE  128
#endif  /* BATCH_SIZE */

#ifndef NPARTIFLOW
#define NPARTIFLOW  128
#endif  /* NPARTIFLOW */

#ifndef NRSLTCLASS
#define NRSLTCLASS  10
#endif  /* NRSLTCLASS */

namespace Ort {

// external classes
struct Value;
struct MemoryInfo;

}

class ADTensor {

private:
  Feature *data;
  int64_t size;
  int64_t *shape;
  int64_t shape_size;

public:
  ADTensor(const int64_t *shape_in, int64_t shape_size_in);
  ADTensor(ADTensor &&) noexcept;
  virtual ~ADTensor();
  
  Ort::Value tensor(const Ort::MemoryInfo &);
  Feature *get_data() { return data; }

};

class ADPFTensor : public ADTensor {

public:
  ADPFTensor(int64_t nfeat);

};

class ADCCTensor : public ADTensor {

public:
  ADCCTensor(int64_t nclss = NRSLTCLASS);

};
