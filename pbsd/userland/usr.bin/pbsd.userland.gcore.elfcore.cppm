module;

export module pbsd.userland.gcore.elfcore;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gcore/elfcore.c
export namespace pbsd::userland::usr_bin::gcore::elfcore {

[[nodiscard]] inline bool elfcore_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::gcore::elfcore
