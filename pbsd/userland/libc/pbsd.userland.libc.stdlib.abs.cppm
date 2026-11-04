module;

export module pbsd.userland.libc.stdlib.abs;

/// abs/labs/llabs from hbsd/src/lib/libc/stdlib/abs.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int abs(int j) noexcept { return j < 0 ? -j : j; }

[[nodiscard]] inline long labs(long j) noexcept { return j < 0 ? -j : j; }

[[nodiscard]] inline long long llabs(long long j) noexcept { return j < 0 ? -j : j; }

} // namespace pbsd::userland::libc
