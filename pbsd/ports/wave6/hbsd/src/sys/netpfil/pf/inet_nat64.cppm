export module pbsd.port.wave6.hbsd.src.sys.netpfil.pf.inet_nat64;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/pf/inet_nat64.c
// void inet_nat64_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/pf/inet_nat64.c wave=wave6 loc=204
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::inet_nat64 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::inet_nat64
