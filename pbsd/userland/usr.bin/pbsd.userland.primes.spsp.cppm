module;

export module pbsd.userland.primes.spsp;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/primes/spsp.c
export namespace pbsd::userland::usr_bin::primes::spsp {

[[nodiscard]] inline bool spsp_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::primes::spsp
