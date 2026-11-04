module;

export module pbsd.userland.primes.pr_tbl;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/primes/pr_tbl.c
export namespace pbsd::userland::usr_bin::primes::pr_tbl {

[[nodiscard]] inline bool pr_tbl_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::primes::pr_tbl
