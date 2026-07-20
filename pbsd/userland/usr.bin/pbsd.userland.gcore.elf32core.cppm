module;

export module pbsd.userland.gcore.elf32core;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gcore/elf32core.c
export namespace pbsd::userland::usr_bin::gcore::elf32core {

[[nodiscard]] inline bool elf32core_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::gcore::elf32core
