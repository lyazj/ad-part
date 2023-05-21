#pragma once

#include <stddef.h>

class TObject;
class TTree;
class TBranch;
class TClonesArray;

class ADBranchBase {

protected:
  TBranch *branch;
  TClonesArray *data;

public:
  ADBranchBase(TTree *tree, const char *brname, const char *tpname);
  ~ADBranchBase();

  TBranch *get_branch() const { return branch; }
  TClonesArray *get_data() const { return data; }
  size_t size() const;
  TObject *&operator[](size_t) const;

};

template<class BranchInfo>
class ADBranch : protected ADBranchBase {

public:
  ADBranch(TTree *tree, BranchInfo = { })  // for CTAD since C++17
    : ADBranchBase(tree, BranchInfo::name, BranchInfo::type::name)
  {
    // do nothing
  }

  using ADBranchBase::get_branch;
  using ADBranchBase::get_data;
  using ADBranchBase::size;

  typename BranchInfo::type::type *&operator[](size_t i) const {
    return (typename BranchInfo::type::type *&)ADBranchBase::operator[](i);
  }

};

// pre-C++17 interface
template<class BranchInfo>
ADBranch<BranchInfo> get_branch(TTree *tree, BranchInfo = { })
{
  return {tree};
}
