module;

export module pbsd.userland.ls.util;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/ls/util.c
export namespace pbsd::userland::bin::ls::util {

[[nodiscard]] inline bool util_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::ls::util
