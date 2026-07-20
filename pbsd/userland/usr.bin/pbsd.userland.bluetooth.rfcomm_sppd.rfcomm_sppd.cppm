module;

export module pbsd.userland.bluetooth.rfcomm_sppd.rfcomm_sppd;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/bluetooth/rfcomm_sppd/rfcomm_sppd.c
export namespace pbsd::userland::usr_bin::bluetooth::rfcomm_sppd::rfcomm_sppd {

[[nodiscard]] inline bool rfcomm_sppd_rfcomm_sppd_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::bluetooth::rfcomm_sppd::rfcomm_sppd
