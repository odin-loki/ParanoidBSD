module;

export module pbsd.userland.gprof.dfn;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gprof/dfn.c
export namespace pbsd::userland::usr_bin::gprof::dfn {

[[nodiscard]] inline bool dfn_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::gprof::dfn
