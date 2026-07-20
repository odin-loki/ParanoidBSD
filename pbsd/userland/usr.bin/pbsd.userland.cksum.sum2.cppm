module;

export module pbsd.userland.cksum.sum2;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/cksum/sum2.c
export namespace pbsd::userland::usr_bin::cksum::sum2 {

[[nodiscard]] inline bool sum2_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::cksum::sum2
