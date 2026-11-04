module;

export module pbsd.userland.patch.util;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/patch/util.c
export namespace pbsd::userland::usr_bin::patch::util {

[[nodiscard]] inline bool util_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::patch::util
