module;

export module pbsd.userland.gprof.printlist;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gprof/printlist.c
export namespace pbsd::userland::usr_bin::gprof::printlist {

[[nodiscard]] inline bool printlist_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::gprof::printlist
