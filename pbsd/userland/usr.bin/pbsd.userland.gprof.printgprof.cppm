module;

export module pbsd.userland.gprof.printgprof;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gprof/printgprof.c
export namespace pbsd::userland::usr_bin::gprof::printgprof {

[[nodiscard]] inline bool printgprof_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::gprof::printgprof
