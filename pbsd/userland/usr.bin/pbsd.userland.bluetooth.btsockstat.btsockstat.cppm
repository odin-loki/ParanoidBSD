module;

export module pbsd.userland.bluetooth.btsockstat.btsockstat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/bluetooth/btsockstat/btsockstat.c
export namespace pbsd::userland::usr_bin::bluetooth::btsockstat::btsockstat {

[[nodiscard]] inline bool btsockstat_btsockstat_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::bluetooth::btsockstat::btsockstat
