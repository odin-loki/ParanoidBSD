export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfilter.netinet.ip_rules;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/ipfilter/netinet/ip_rules.c
// void ip_rules_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfilter/netinet/ip_rules.c wave=wave6 loc=269
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::ip_rules {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfilter::netinet::ip_rules
