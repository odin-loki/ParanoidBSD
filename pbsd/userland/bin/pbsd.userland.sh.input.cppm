module;

export module pbsd.userland.sh.input;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/input.c
export namespace pbsd::userland::bin::sh::input {

[[nodiscard]] inline bool input_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::input
