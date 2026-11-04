module;

export module pbsd.userland.sort.vsort;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/sort/vsort.c
export namespace pbsd::userland::usr_bin::sort::vsort {

[[nodiscard]] inline bool vsort_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::sort::vsort
