module;

export module pbsd.userland.mkimg.mbr;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkimg/mbr.c
export namespace pbsd::userland::usr_bin::mkimg::mbr {

[[nodiscard]] inline bool mbr_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkimg::mbr
