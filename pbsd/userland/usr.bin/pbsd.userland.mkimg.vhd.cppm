module;

export module pbsd.userland.mkimg.vhd;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkimg/vhd.c
export namespace pbsd::userland::usr_bin::mkimg::vhd {

[[nodiscard]] inline bool vhd_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkimg::vhd
