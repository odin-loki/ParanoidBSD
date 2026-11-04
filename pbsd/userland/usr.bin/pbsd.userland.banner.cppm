module;

export module pbsd.userland.banner;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/banner/banner.c
export namespace pbsd::userland::usr_bin::banner {


inline constexpr const char* kOptString = "d:wt:";


} // namespace pbsd::userland::usr_bin::banner
