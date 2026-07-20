module;

export module pbsd.userland.sh.miscbltin;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/miscbltin.c
export namespace pbsd::userland::bin::sh::miscbltin {

[[nodiscard]] inline bool miscbltin_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::miscbltin
