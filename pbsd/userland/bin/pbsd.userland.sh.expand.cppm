module;

export module pbsd.userland.sh.expand;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/expand.c
export namespace pbsd::userland::bin::sh::expand {

[[nodiscard]] inline bool expand_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::expand
