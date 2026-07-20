export module pbsd.port.wave9.hbsd.src.contrib.libpcap.msdos.pktdrvr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/msdos/pktdrvr.c
// void pktdrvr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/msdos/pktdrvr.c wave=wave9 loc=1436
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::msdos::pktdrvr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::msdos::pktdrvr
