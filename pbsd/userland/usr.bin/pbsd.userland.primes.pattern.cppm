module;

export module pbsd.userland.primes.pattern;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/primes/pattern.c
export namespace pbsd::userland::usr_bin::primes::pattern {

[[nodiscard]] inline bool pattern_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::primes::pattern
