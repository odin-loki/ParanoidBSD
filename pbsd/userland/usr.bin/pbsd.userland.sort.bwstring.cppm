module;

export module pbsd.userland.sort.bwstring;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/sort/bwstring.c
export namespace pbsd::userland::usr_bin::sort::bwstring {

[[nodiscard]] inline bool bwstring_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::sort::bwstring
