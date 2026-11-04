module;

export module pbsd.userland.whereis;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/whereis/whereis.c
export namespace pbsd::userland::usr_bin::whereis {


inline constexpr const char* kOptString = "bB:mMsu";


} // namespace pbsd::userland::usr_bin::whereis
