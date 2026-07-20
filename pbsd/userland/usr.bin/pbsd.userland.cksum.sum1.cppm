module;

export module pbsd.userland.cksum.sum1;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/cksum/sum1.c
export namespace pbsd::userland::usr_bin::cksum::sum1 {

[[nodiscard]] inline bool sum1_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::cksum::sum1
