module;

export module pbsd.userland.grep.util;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/grep/util.c
export namespace pbsd::userland::usr_bin::grep::util {

[[nodiscard]] inline bool util_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::grep::util
