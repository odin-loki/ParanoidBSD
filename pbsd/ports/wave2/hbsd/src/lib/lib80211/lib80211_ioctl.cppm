export module pbsd.port.wave2.hbsd.src.lib.lib80211.lib80211_ioctl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/lib80211/lib80211_ioctl.c
// void lib80211_ioctl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/lib80211/lib80211_ioctl.c wave=wave2 loc=157
export namespace pbsd::port::wave2::hbsd::src::lib::lib80211::lib80211_ioctl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::lib80211::lib80211_ioctl
