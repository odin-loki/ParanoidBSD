module;

export module pbsd.userland.libc.stdlib.labs;

export import pbsd.core;

/// labs from hbsd/src/lib/libc/stdlib/labs.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long labs_val(long j) noexcept { return j < 0 ? -j : j; }

} // namespace pbsd::userland::libc
