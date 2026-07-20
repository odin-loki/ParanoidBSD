export module pbsd.port.wave6.hbsd.src.sys.netinet6.ip6_forward;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet6/ip6_forward.c
// void ip6_forward_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet6/ip6_forward.c wave=wave6 loc=477
export namespace pbsd::port::wave6::hbsd::src::sys::netinet6::ip6_forward {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet6::ip6_forward
