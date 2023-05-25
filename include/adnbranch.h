#pragma once

#include <stddef.h>

class TObject;
class TTree;
class TBranch;

class ADNBranchBase {

protected:
  TBranch *branch;
  size_t mbsize;
  size_t curpos;

public:
  ADNBranchBase(TTree *tree, const char *brname, size_t mbsize);
  ~ADNBranchBase();

  TBranch *get_branch() const { return branch; }
  size_t get_entry() const { return curpos; }

  size_t get_capacity() const;
  void reserve(size_t nmb);
  void *get_data() const;

  size_t read_entry(size_t i);
  size_t read_entry_once(size_t i);

};

template<class NBranchInfo, class DBranchInfo>
class ADNBranch : protected ADNBranchBase {

protected:
  ADNBranchBase data;
  size_t mbnumb;

public:
  ADNBranch(TTree *tree)
    : ADNBranchBase(tree, NBranchInfo::name, sizeof(typename NBranchInfo::type::type)),
      data(tree, DBranchInfo::name, sizeof(typename DBranchInfo::type::type))
  {
    ADNBranchBase::reserve(1);
    mbnumb = 0;
  }

  TBranch *get_nbranch() const { return get_branch(); }
  TBranch *get_dbranch() const { return data.get_branch(); }
  size_t get_capacity() const { return data.get_capacity(); }
  size_t get_nentry() const { return get_entry(); }
  size_t get_dentry() const { return data.get_entry(); }

  void reserve(size_t nmb) { data.reserve(nmb); }

  size_t read_entry(size_t i) {
    ADNBranchBase::read_entry_once(i);
    mbnumb = *(typename NBranchInfo::type::type *)get_data();
    reserve(mbnumb);
    return data.read_entry(i);
  }

  typename DBranchInfo::type::type &operator[](size_t i) const {
    return ((typename DBranchInfo::type::type *)data.get_data())[i];
  }

  size_t size() const { return mbnumb; }

};

// pre-C++17 interface
template<class NBranchInfo, class DBranchInfo>
ADNBranch<NBranchInfo, DBranchInfo>
get_nbranch(TTree *tree, NBranchInfo = { }, DBranchInfo = { })
{
  return {tree};
}
