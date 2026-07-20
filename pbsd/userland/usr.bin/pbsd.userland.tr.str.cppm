module;

export module pbsd.userland.tr.str;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tr/str.c
export namespace pbsd::userland::usr_bin::tr::str {

[[nodiscard]] inline bool str_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tr::str
