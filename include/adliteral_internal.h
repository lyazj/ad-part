#pragma once

template<class C, C... CPack>
struct ADCharPack {
  static constexpr const C str[sizeof...(CPack) + 1] = {CPack..., 0};
};

template<class Type, class Name>  // Name: ADCharPack
struct ADTypeInfo {
  static constexpr const char *name = Name::str;
  typedef Type type;
};

template<class Info, class Name>
struct ADBranchInfo {
  static constexpr const char *name = Name::str;
  typedef Info TypeInfo;
};

template<class Name>  // Name: ADCharPack
auto get_type_info(Name = { });  // -> ADTypeInfo

template<class Name>  // Name: ADCharPack
auto get_branch_type_info(Name = { });  // -> ADTypeInfo

template<class Name>  // Name: ADCharPack
auto get_branch_info(Name name = { })  // -> ADBranchInfo
{
  return ADBranchInfo<decltype(get_branch_type_info(name)), Name>();
}
