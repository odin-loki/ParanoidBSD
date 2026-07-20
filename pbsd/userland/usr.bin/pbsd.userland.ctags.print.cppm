module;

export module pbsd.userland.ctags.print;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ctags/print.c
export namespace pbsd::userland::usr_bin::ctags::print {

[[nodiscard]] inline bool print_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::ctags::print
