export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.nptv6.nptv6;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfw/nptv6/nptv6.c
// void nptv6_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/nptv6/nptv6.c wave=wave6 loc=1040
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::nptv6::nptv6 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::nptv6::nptv6
