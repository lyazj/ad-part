#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wgnu-string-literal-operator-template"

template<class C, C... CPack>
struct ADCharPack {
  static constexpr const C str[sizeof...(CPack) + 1] = {CPack..., 0};
};

// Required before C++17.
template<class C, C... CPack>
constexpr const C ADCharPack<C, CPack...>::str[sizeof...(CPack) + 1];

// Get ADCharPack literal.
template<class C, C... CPack>
constexpr ADCharPack<C, CPack...> operator""_pack()
{
  return { };
}

template<class Name>
struct ADTypeInfo;

/*
 * Register a C++ type.
 *
 *   name_in: ADCharPack, type name in literal.
 *   type_in: type, the real C++ type.
 */
#define register_type(name_in, type_in) \
  template<> \
  struct ADTypeInfo<decltype(name_in)> { \
    static constexpr const char *name = name_in.str; \
    typedef type_in type; \
  }

// ----------------------------------------------------------------------
// User registered types:
//   e.g. register_type("int"_pack, int);

// ----------------------------------------------------------------------

/*
 * Register a C++ class with forward declaration.
 *
 *   name_in: ADCharPack, class name in literal.
 *   type_in: class, the real C++ class.
 */
#define register_class(name_in, type_in) \
  class type_in; \
  register_type(name_in, type_in)

// ----------------------------------------------------------------------
// User registered classes:
//   e.g. register_type("Jet"_pack, Jet);

// Delphes defined classes.
register_class("Event"_pack, Event);
register_class("LHCOEvent"_pack, LHCOEvent);
register_class("LHEFEvent"_pack, LHEFEvent);
register_class("LHEFWeight"_pack, LHEFWeight);
register_class("HepMCEvent"_pack, HepMCEvent);
register_class("GenParticle"_pack, GenParticle);
register_class("Vertex"_pack, Vertex);
register_class("MissingET"_pack, MissingET);
register_class("ScalarHT"_pack, ScalarHT);
register_class("Rho"_pack, Rho);
register_class("Weight"_pack, Weight);
register_class("Photon"_pack, Photon);
register_class("Electron"_pack, Electron);
register_class("Muon"_pack, Muon);
register_class("Jet"_pack, Jet);
register_class("Track"_pack, Track);
register_class("Tower"_pack, Tower);
register_class("ParticleFlowCandidate"_pack, ParticleFlowCandidate);
register_class("HectorHit"_pack, HectorHit);
register_class("Candidate"_pack, Candidate);

// ----------------------------------------------------------------------

// Get registered ADTypeInfo literal.
template<class C, C... CPack>
constexpr ADTypeInfo<ADCharPack<C, CPack...>> operator""_type()
{
  return { };
}

template<class Name>
struct ADBranchInfo;

/*
 * Register a branch.
 *
 *   name_in: ADCharPack, branch name.
 *   type_in: ADTypeInfo, describing data type.
 */
#define register_branch(name_in, type_in) \
  template<> \
  struct ADBranchInfo<decltype(name_in)> { \
    static constexpr const char *name = name_in.str; \
    typedef decltype(type_in) type; \
  }

// ----------------------------------------------------------------------
// User registered branches:
//   e.g. register_type("GenJet"_pack, "Jet"_type);

register_branch("Jet"_pack, "Jet"_type);
register_branch("FatJet"_pack, "Jet"_type);

// ----------------------------------------------------------------------

// Get registered ADBranchInfo literal.
template<class C, C... CPack>
constexpr ADBranchInfo<ADCharPack<C, CPack...>> operator""_branch()
{
  return { };
}

#pragma GCC diagnostic pop
