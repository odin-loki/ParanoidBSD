export module pbsd.port.wave4.hbsd.src.sys.compat.linuxkpi.common.src.linux_80211;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linuxkpi/common/src/linux_80211.c
// void linux_80211_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linuxkpi/common/src/linux_80211.c wave=wave4 loc=10165
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_80211 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_80211
