module;

export module pbsd.userland.netstat.unix;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/netstat/unix.c
export namespace pbsd::userland::usr_bin::netstat::unix {

[[nodiscard]] inline bool unix_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::netstat::unix
