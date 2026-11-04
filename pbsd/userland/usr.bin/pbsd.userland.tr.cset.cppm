module;

export module pbsd.userland.tr.cset;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tr/cset.c
export namespace pbsd::userland::usr_bin::tr::cset {

[[nodiscard]] inline bool cset_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tr::cset
