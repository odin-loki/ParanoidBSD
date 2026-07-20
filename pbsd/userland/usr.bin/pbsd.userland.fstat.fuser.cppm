module;

export module pbsd.userland.fstat.fuser;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/fstat/fuser.c
export namespace pbsd::userland::usr_bin::fstat::fuser {

[[nodiscard]] inline bool fuser_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::fstat::fuser
