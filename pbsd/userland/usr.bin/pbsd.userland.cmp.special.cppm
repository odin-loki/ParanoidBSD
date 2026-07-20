module;

export module pbsd.userland.cmp.special;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/cmp/special.c
export namespace pbsd::userland::usr_bin::cmp::special {

[[nodiscard]] inline bool special_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::cmp::special
