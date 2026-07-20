module;

export module pbsd.userland.finger.lprint;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/finger/lprint.c
export namespace pbsd::userland::usr_bin::finger::lprint {

[[nodiscard]] inline bool lprint_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::finger::lprint
