module;

export module pbsd.userland.cksum.print;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/cksum/print.c
export namespace pbsd::userland::usr_bin::cksum::print {

[[nodiscard]] inline bool print_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::cksum::print
