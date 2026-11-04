module;

export module pbsd.userland.bluetooth.rfcomm_sppd.rfcomm_sdp;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/bluetooth/rfcomm_sppd/rfcomm_sdp.c
export namespace pbsd::userland::usr_bin::bluetooth::rfcomm_sppd::rfcomm_sdp {

[[nodiscard]] inline bool rfcomm_sppd_rfcomm_sdp_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::bluetooth::rfcomm_sppd::rfcomm_sdp
