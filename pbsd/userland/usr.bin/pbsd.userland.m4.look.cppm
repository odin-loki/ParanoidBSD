module;

export module pbsd.userland.m4.look;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/m4/look.c
export namespace pbsd::userland::usr_bin::m4::look {

[[nodiscard]] inline bool look_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::m4::look
