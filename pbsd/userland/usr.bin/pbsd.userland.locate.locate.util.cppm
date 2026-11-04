module;

export module pbsd.userland.locate.locate.util;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/locate/locate/util.c
export namespace pbsd::userland::usr_bin::locate::locate::util {

[[nodiscard]] inline bool locate_util_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::locate::locate::util
