module;

export module pbsd.userland.sort.coll;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/sort/coll.c
export namespace pbsd::userland::usr_bin::sort::coll {

[[nodiscard]] inline bool coll_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::sort::coll
