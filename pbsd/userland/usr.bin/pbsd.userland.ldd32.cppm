module;

export module pbsd.userland.ldd32;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ldd32/ldd32.c
export namespace pbsd::userland::usr_bin::ldd32 {

[[nodiscard]] inline bool ldd32_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::ldd32
