module;

export module pbsd.userland.mkimg.bsd;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkimg/bsd.c
export namespace pbsd::userland::usr_bin::mkimg::bsd {

[[nodiscard]] inline bool bsd_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkimg::bsd
