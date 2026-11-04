module;

export module pbsd.userland.systat.sctp;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/sctp.c
export namespace pbsd::userland::usr_bin::systat::sctp {

[[nodiscard]] inline bool sctp_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::sctp
