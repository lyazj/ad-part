#include "adbranch.h"
#include <TTree.h>
#include <TBranch.h>
#include <TClonesArray.h>

using namespace std;

ADBranchBase::ADBranchBase(TTree *tree_in, const char *brname, const char *tpname)
{
  tree = tree_in;
  branch = tree->GetBranch(brname);
  data = new TClonesArray(tpname);
  branch->SetAddress(&data);
  branch->SetStatus(1);
}

ADBranchBase::~ADBranchBase()
{
  branch->SetAddress(NULL);
  branch->SetStatus(0);
  delete data;
}

TObject *&ADBranchBase::operator[](size_t i) const
{
  return data->operator[](i);
}
