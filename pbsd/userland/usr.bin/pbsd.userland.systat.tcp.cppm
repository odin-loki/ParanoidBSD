module;

export module pbsd.userland.systat.tcp;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/tcp.c
export namespace pbsd::userland::usr_bin::systat::tcp {

[[nodiscard]] inline bool tcp_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::tcp
