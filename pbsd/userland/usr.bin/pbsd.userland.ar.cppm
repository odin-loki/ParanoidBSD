module;

export module pbsd.userland.ar;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/ar/ar.c
export namespace pbsd::userland::usr_bin::ar {


inline constexpr const char* kOptString = "abcdD:mopqrstuvx";


} // namespace pbsd::userland::usr_bin::ar
