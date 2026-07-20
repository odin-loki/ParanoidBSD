module;

export module pbsd.userland.cp.utils;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/cp/utils.c
export namespace pbsd::userland::bin::cp::utils {

[[nodiscard]] inline bool utils_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::cp::utils
