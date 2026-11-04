module;

export module pbsd.userland.diff.xmalloc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/diff/xmalloc.c
export namespace pbsd::userland::usr_bin::diff::xmalloc {

[[nodiscard]] inline bool xmalloc_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::diff::xmalloc
