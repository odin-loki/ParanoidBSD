module;

export module pbsd.userland.m4.gnum4;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/m4/gnum4.c
export namespace pbsd::userland::usr_bin::m4::gnum4 {

[[nodiscard]] inline bool gnum4_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::m4::gnum4
