module;

export module pbsd.userland.nfsiod;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/sbin/nfsiod/nfsiod.c
export namespace pbsd::userland::sbin::nfsiod {


inline constexpr int kDefaultDaemons = 4;


} // namespace pbsd::userland::sbin::nfsiod
