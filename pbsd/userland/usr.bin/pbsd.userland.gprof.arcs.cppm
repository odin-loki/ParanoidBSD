module;

export module pbsd.userland.gprof.arcs;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gprof/arcs.c
export namespace pbsd::userland::usr_bin::gprof::arcs {

[[nodiscard]] inline bool arcs_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::gprof::arcs
