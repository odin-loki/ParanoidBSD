module;

export module pbsd.userland.sh.options;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/options.c
export namespace pbsd::userland::bin::sh::options {

[[nodiscard]] inline bool options_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::options
