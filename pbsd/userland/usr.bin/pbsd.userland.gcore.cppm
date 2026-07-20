module;

export module pbsd.userland.gcore;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gcore/gcore.c
export namespace pbsd::userland::usr_bin::gcore {

[[nodiscard]] inline bool gcore_force(char flag) noexcept { return flag == 'f'; }

} // namespace pbsd::userland::usr_bin::gcore
