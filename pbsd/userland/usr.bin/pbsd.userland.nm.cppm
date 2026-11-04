module;

export module pbsd.userland.nm;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/nm/nm.c
export namespace pbsd::userland::usr_bin::nm {


inline constexpr const char* kOptString = "AaC:D:egj:n:oprsu:x";


} // namespace pbsd::userland::usr_bin::nm
