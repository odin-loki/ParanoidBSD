module;

export module pbsd.userland.stty.print;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/stty/print.c
export namespace pbsd::userland::bin::stty::print {

[[nodiscard]] inline bool print_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::stty::print
