#include "admem.h"
#include <unordered_map>
#include <new>
#include <stdexcept>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

class ADAllocator::Driver {

public:
  unordered_map<void *, size_t> allocated;

};

ADAllocator::ADAllocator()
{
  driver = new Driver;
}

ADAllocator::~ADAllocator()
{
  if(!driver->allocated.empty()) {
    fprintf(stderr, "WARNING: unfreed memory detected\n");
  }
  delete driver;
}

size_t ADAllocator::size(void *addr)
{
  if(addr == NULL) return 0;
  return driver->allocated.at(addr);
}

void *ADAllocator::alloc(size_t n)
{
  if(n == 0) return NULL;
  void *addr = malloc(n);
  if(addr == NULL) throw bad_alloc();
  if(!driver->allocated.emplace(addr, n).second) {
    free(addr);
    throw runtime_error("duplicated address");
  }
  fprintf(stderr, "INFO: %s: %p (%zd bytes)\n", __func__, addr, n);
  return addr;
}

void ADAllocator::dealloc(void *addr)
{
  if(addr == NULL) return;
  if(!driver->allocated.erase(addr)) {
    throw runtime_error("not AD-allocated");
  }
  char buf[64];  // This avoids use-after-free warning.
  sprintf(buf, "INFO: %s: %p\n", __func__, addr);
  free(addr);
  fprintf(stderr, "%s", buf);
}

void *ADAllocator::realloc(void *addr, size_t n, bool copy)
{
  size_t n_old = size(addr);
  void *addr_new = alloc(n);
  if(copy) {
    memcpy(addr_new, addr, min(n, n_old));
  }
  dealloc(addr);
  return addr_new;
}

void *ADAllocator::extend(void *addr, size_t n, bool copy)
{
  size_t n_old = size(addr);
  if(n_old >= n) return addr;
  return realloc(addr, n, copy);
}

void *ADAllocator::shrink(void *addr, size_t n, bool copy)
{
  size_t n_old = size(addr);
  if(n_old <= n) return addr;
  return realloc(addr, n, copy);
}
