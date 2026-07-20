module;

export module pbsd.userland.mkimg.vhdx;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkimg/vhdx.c
export namespace pbsd::userland::usr_bin::mkimg::vhdx {

[[nodiscard]] inline bool vhdx_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkimg::vhdx
