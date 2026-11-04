module;

export module pbsd.userland.sh.main;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/main.c
export namespace pbsd::userland::bin::sh::main {

[[nodiscard]] inline bool main_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::main
