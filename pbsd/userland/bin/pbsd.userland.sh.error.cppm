module;

export module pbsd.userland.sh.error;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/error.c
export namespace pbsd::userland::bin::sh::error {

[[nodiscard]] inline bool error_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::error
