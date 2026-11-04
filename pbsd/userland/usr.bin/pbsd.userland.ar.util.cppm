module;

export module pbsd.userland.ar.util;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ar/util.c
export namespace pbsd::userland::usr_bin::ar::util {

[[nodiscard]] inline bool util_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::ar::util
