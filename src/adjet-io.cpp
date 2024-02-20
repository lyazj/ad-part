#include "addef.h"
#include "adjet.h"
#include <stdexcept>
#include <zlib.h>
#include <string.h>

using namespace std;

namespace {

template<class T>
void write_features(const T &obj, gzFile file)
{
  size_t s = obj.feature_end - obj.feature_begin;
  //size_t r = gzfwrite(obj.feature_begin, 1, s, file);
  size_t r = gzwrite(file, obj.feature_begin, s);
  if(r != s) {
    int e;
    throw runtime_error(gzerror(file, &e));
  }
}

template<class T>
bool read_features(T &obj, gzFile file)
{
  size_t s = obj.feature_end - obj.feature_begin;
  //size_t r = gzfread(obj.feature_begin, 1, s, file);
  size_t r = gzread(file, obj.feature_begin, s);
  if(r != s) {
    int e;
    const char *msg = gzerror(file, &e);
    if(e || r) {
      throw runtime_error(msg);
    }
    return false;
  }
  return true;
}

}  // namespace

ADParticle::ADParticle()
{
  memset(feature_begin, 0, feature_end - feature_begin);  // padding
}

bool ADParticle::read(gzFile file)
{
  return read_features(*this, file);
}

void ADParticle::write(gzFile file) const
{
  write_features(*this, file);
}

ADJet::ADJet()
{
  par = (ADParticle *)malloc(NPARTIFLOW * sizeof(*par));
  if(par == NULL) throw bad_alloc();
}

ADJet::~ADJet()
{
  for(size_t i = 0; i < NPARTIFLOW; ++i) {
    par[i].~ADParticle();
  }
  free(par);
}

bool ADJet::read(gzFile file)
{
  if(!read_features(*this, file)) return false;
  for(size_t i = 0; i < NPARTIFLOW; ++i) {
    if(!par[i].read(file)) {
      throw runtime_error("unexpected EOF");
    }
  }
  return true;
}

bool ADJet::read_without_particles(gzFile file)
{
  return read_features(*this, file);
}

void ADJet::write(gzFile file) const
{
  write_features(*this, file);
  for(size_t i = 0; i < NPARTIFLOW; ++i) {
    par[i].write(file);
  }
}

void ADJet::write_without_particles(gzFile file) const
{
  write_features(*this, file);
}

bool ADParTOutput::read(gzFile file)
{
  return read_features(*this, file);
}

void ADParTOutput::write(gzFile file) const
{
  write_features(*this, file);
}

bool ADParTHidden::read(gzFile file)
{
  return read_features(*this, file);
}

void ADParTHidden::write(gzFile file) const
{
  write_features(*this, file);
}

bool ADEvent::read(gzFile file)
{
  return read_features(*this, file);
}

void ADEvent::write(gzFile file) const
{
  write_features(*this, file);
}

ADLepton::ADLepton()
{
  memset(feature_begin, 0, feature_end - feature_begin);  // padding
}

bool ADLepton::read(gzFile file)
{
  return read_features(*this, file);
}

void ADLepton::write(gzFile file) const
{
  write_features(*this, file);
}
