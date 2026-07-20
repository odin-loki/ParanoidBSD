module;

export module pbsd.userland.ps.fmt;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/ps/fmt.c
export namespace pbsd::userland::bin::ps::fmt {

[[nodiscard]] inline bool fmt_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::ps::fmt
