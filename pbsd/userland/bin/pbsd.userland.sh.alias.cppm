module;

export module pbsd.userland.sh.alias;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/alias.c
export namespace pbsd::userland::bin::sh::alias {

[[nodiscard]] inline bool alias_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::alias
