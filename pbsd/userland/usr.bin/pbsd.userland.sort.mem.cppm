module;

export module pbsd.userland.sort.mem;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/sort/mem.c
export namespace pbsd::userland::usr_bin::sort::mem {

[[nodiscard]] inline bool mem_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::sort::mem
