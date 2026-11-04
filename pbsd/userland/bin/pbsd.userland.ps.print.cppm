module;

export module pbsd.userland.ps.print;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/ps/print.c
export namespace pbsd::userland::bin::ps::print {

[[nodiscard]] inline bool print_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::ps::print
