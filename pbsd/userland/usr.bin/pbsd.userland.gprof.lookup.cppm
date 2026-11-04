module;

export module pbsd.userland.gprof.lookup;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gprof/lookup.c
export namespace pbsd::userland::usr_bin::gprof::lookup {

[[nodiscard]] inline bool lookup_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::gprof::lookup
