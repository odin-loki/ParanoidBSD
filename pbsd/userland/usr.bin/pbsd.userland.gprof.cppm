module;

export module pbsd.userland.gprof;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gprof/gprof.c
export namespace pbsd::userland::usr_bin::gprof {

[[nodiscard]] inline bool gprof_flat(char flag) noexcept { return flag == 'f'; }

} // namespace pbsd::userland::usr_bin::gprof
