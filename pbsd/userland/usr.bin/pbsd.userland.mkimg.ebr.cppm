module;

export module pbsd.userland.mkimg.ebr;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkimg/ebr.c
export namespace pbsd::userland::usr_bin::mkimg::ebr {

[[nodiscard]] inline bool ebr_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkimg::ebr
