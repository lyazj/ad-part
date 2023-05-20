#pragma once

#include "adliteral_internal.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wgnu-string-literal-operator-template"

// ADCharPack literal
template<class C, C... CPack>
auto operator""_pack()
{
  return ADCharPack<C, CPack...>();
}

// ADTypeInfo literal from type name
template<class C, C... CPack>
auto operator""_type()
{
  return get_type_info(operator""_pack<C, CPack...>());
}

// ADTypeInfo literal from branch name
template<class C, C... CPack>
auto operator""_branch_type()
{
  return get_branch_type_info(operator""_pack<C, CPack...>());
}

// ADBranchInfo literal from branch name
template<class C, C... CPack>
auto operator""_branch()
{
  return get_branch_info(operator""_pack<C, CPack...>());
}

// name: ADCharPack  type: any C++ type
#define register_type_nodecl(name, type) \
  template<> \
  auto get_type_info(decltype(name)) \
  { \
    return ADTypeInfo<type, decltype(name)>(); \
  } \
  template<class Name> \
  auto get_type_info(Name)

// name: ADCharPack  type: class declared in this scpoe
#define register_type(name, type) \
  class type; \
  register_type_nodecl(name, type)

// name: ADCharPack  type: ADTypeInfo
#define register_branch_type(name, type) \
  template<> \
  auto get_branch_type_info(decltype(name)) \
  { \
    return type; \
  } \
  template<class Name> \
  auto get_branch_type_info(Name)

register_type("Jet"_pack, Jet);
register_branch_type("Jet"_pack, "Jet"_type);

#pragma GCC diagnostic pop
