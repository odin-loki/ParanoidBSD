module;

export module pbsd.userland.finger.util;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/finger/util.c
export namespace pbsd::userland::usr_bin::finger::util {

[[nodiscard]] inline bool util_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::finger::util
