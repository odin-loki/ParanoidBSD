module;

export module pbsd.userland.ls.print;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/ls/print.c
export namespace pbsd::userland::bin::ls::print {

[[nodiscard]] inline bool print_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::ls::print
