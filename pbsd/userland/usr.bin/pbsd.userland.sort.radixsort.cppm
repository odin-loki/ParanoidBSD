module;

export module pbsd.userland.sort.radixsort;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/sort/radixsort.c
export namespace pbsd::userland::usr_bin::sort::radixsort {

[[nodiscard]] inline bool radixsort_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::sort::radixsort
