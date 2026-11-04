module;

export module pbsd.userland.gprof.elf;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gprof/elf.c
export namespace pbsd::userland::usr_bin::gprof::elf {

[[nodiscard]] inline bool elf_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::gprof::elf
