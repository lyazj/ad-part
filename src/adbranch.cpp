#include "adbranch.h"
#include <TTree.h>
#include <TBranch.h>
#include <TClonesArray.h>

using namespace std;

ADBranchBase::ADBranchBase(TTree *tree, const char *brname, const char *tpname)
{
  data = new TClonesArray(tpname);
  branch = tree->GetBranch(brname);
  branch->SetAddress(&data);
  branch->SetStatus(1);
}

ADBranchBase::~ADBranchBase()
{
  branch->SetAddress(NULL);
  branch->SetStatus(0);
  delete data;
}

size_t ADBranchBase::size() const
{
  return data->GetEntries();
}

TObject *&ADBranchBase::operator[](size_t i) const
{
  return data->operator[](i);
}
