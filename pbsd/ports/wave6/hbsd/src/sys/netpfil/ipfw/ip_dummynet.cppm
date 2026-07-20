export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.ip_dummynet;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfw/ip_dummynet.c
// void ip_dummynet_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/ip_dummynet.c wave=wave6 loc=2824
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::ip_dummynet {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::ip_dummynet
