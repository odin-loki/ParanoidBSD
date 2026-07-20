module;

export module pbsd.userland.cmp.regular;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/cmp/regular.c
export namespace pbsd::userland::usr_bin::cmp::regular {

[[nodiscard]] inline bool regular_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::cmp::regular
