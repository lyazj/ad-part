#pragma once

#include <stddef.h>

class ADAllocator {

private:
  class Driver;
  Driver *driver;

public:
  ADAllocator();
  ADAllocator(const ADAllocator &) = delete;
  ~ADAllocator();

  size_t size(void *);

  void *alloc(size_t);
  void dealloc(void *);
  void *realloc(void *, size_t, bool copy = false);
  void *extend(void *, size_t, bool copy = false);
  void *shrink(void *, size_t, bool copy = false);

};
