module;

export module pbsd.userland.top.utils;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/top/utils.c
export namespace pbsd::userland::usr_bin::top::utils {

[[nodiscard]] inline bool utils_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::top::utils
