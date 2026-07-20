module;

export module pbsd.userland.netstat.nhgrp;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/netstat/nhgrp.c
export namespace pbsd::userland::usr_bin::netstat::nhgrp {

[[nodiscard]] inline bool nhgrp_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::netstat::nhgrp
