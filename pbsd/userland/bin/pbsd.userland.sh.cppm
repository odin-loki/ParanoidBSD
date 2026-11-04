module;

export module pbsd.userland.sh;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/alias.c
export namespace pbsd::userland::bin::sh {

[[nodiscard]] inline bool sh_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::bin::sh
