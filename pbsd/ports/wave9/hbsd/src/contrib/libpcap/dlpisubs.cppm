export module pbsd.port.wave9.hbsd.src.contrib.libpcap.dlpisubs;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/dlpisubs.c
// void dlpisubs_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/dlpisubs.c wave=wave9 loc=539
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::dlpisubs {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::dlpisubs
