module;

export module pbsd.userland.sh.output;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/output.c
export namespace pbsd::userland::bin::sh::output {

[[nodiscard]] inline bool output_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::output
