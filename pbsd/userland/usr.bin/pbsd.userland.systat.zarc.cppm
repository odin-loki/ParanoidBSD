module;

export module pbsd.userland.systat.zarc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/zarc.c
export namespace pbsd::userland::usr_bin::systat::zarc {

[[nodiscard]] inline bool zarc_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::zarc
