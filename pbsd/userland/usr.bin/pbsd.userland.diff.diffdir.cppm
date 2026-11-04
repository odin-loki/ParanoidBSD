module;

export module pbsd.userland.diff.diffdir;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/diff/diffdir.c
export namespace pbsd::userland::usr_bin::diff::diffdir {

[[nodiscard]] inline bool diffdir_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::diff::diffdir
