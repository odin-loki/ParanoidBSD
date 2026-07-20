module;

export module pbsd.userland.sh.trap;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/trap.c
export namespace pbsd::userland::bin::sh::trap {

[[nodiscard]] inline bool trap_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::trap
