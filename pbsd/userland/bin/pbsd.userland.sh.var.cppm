module;

export module pbsd.userland.sh.var;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/var.c
export namespace pbsd::userland::bin::sh::var {

[[nodiscard]] inline bool var_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::var
