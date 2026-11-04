module;

export module pbsd.userland.truss.syscalls;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/truss/syscalls.c
export namespace pbsd::userland::usr_bin::truss::syscalls {

[[nodiscard]] inline bool syscalls_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::truss::syscalls
