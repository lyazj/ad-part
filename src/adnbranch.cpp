#include "adnbranch.h"
#include "admem.h"
#include <TTree.h>
#include <stdexcept>

using namespace std;

namespace {

ADAllocator brmem;

}  // namespace

ADNBranchBase::ADNBranchBase(TTree *tree,
    const char *brname, size_t mbsize_in)
{
  branch = tree->GetBranch(brname);
  if(branch == NULL) {
    throw runtime_error("error getting branch: "s + brname);
  }
  branch->SetStatus(1);
  mbsize = mbsize_in;
  curpos = (size_t)-1;
}

ADNBranchBase::~ADNBranchBase()
{
  brmem.dealloc(branch->GetAddress());
  branch->SetAddress(NULL);
  branch->SetStatus(0);
}

void *ADNBranchBase::get_data() const
{
  void *addr = branch->GetAddress();
  if(addr == NULL) {
    throw runtime_error("null branch address");
  }
  return addr;
}

size_t ADNBranchBase::read_entry(size_t i)
{
  size_t r = branch->GetEntry(i);
  curpos = i;
  return r;
}

size_t ADNBranchBase::read_entry_once(size_t i)
{
  if(get_entry() == i) return 0;
  return read_entry(i);
}

size_t ADNBranchBase::get_capacity() const
{
  return brmem.size(branch->GetAddress());
}

void ADNBranchBase::reserve(size_t nmb)
{
  void *addr = branch->GetAddress();
  addr = brmem.extend(addr, nmb * mbsize);
  branch->SetAddress(addr);
}
