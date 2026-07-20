module;

export module pbsd.userland.sh.bltin.echo;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/bltin/echo.c
export namespace pbsd::userland::bin::sh::bltin::echo {

[[nodiscard]] inline bool bltin_echo_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::bltin::echo
