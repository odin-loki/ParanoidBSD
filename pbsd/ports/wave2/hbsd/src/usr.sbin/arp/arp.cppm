export module pbsd.port.wave2.hbsd.src.usr_sbin.arp.arp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/arp/arp.c
// void arp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/arp/arp.c wave=wave2 loc=899
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::arp::arp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::arp::arp
