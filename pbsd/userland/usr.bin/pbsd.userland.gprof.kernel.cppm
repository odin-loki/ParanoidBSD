module;

export module pbsd.userland.gprof.kernel;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gprof/kernel.c
export namespace pbsd::userland::usr_bin::gprof::kernel {

[[nodiscard]] inline bool kernel_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::gprof::kernel
