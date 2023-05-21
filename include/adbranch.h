#pragma once

#include <stddef.h>

class TObject;
class TTree;
class TBranch;
class TClonesArray;

class ADBranchBase {

protected:
  TTree *tree;
  TBranch *branch;
  TClonesArray *data;

public:
  ADBranchBase(TTree *tree_in, const char *brname, const char *tpname);
  ~ADBranchBase();

  TBranch *get_branch() const { return branch; }
  TClonesArray *get_data() const { return data; }
  TObject *&operator[](size_t) const;

};

template<class BranchInfo>
class ADBranch : protected ADBranchBase {

public:
  ADBranch(TTree *tree_in, BranchInfo = { })
    : ADBranchBase(tree_in, BranchInfo::name, BranchInfo::type::name)
  {
    // do nothing
  }

  using ADBranchBase::get_branch;
  using ADBranchBase::get_data;

  typename BranchInfo::type::type *&operator[](size_t i) const {
    return (typename BranchInfo::type::type *&)ADBranchBase::operator[](i);
  }

};
