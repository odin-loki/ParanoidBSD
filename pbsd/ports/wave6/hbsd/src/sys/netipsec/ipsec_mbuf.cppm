export module pbsd.port.wave6.hbsd.src.sys.netipsec.ipsec_mbuf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netipsec/ipsec_mbuf.c
// void ipsec_mbuf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netipsec/ipsec_mbuf.c wave=wave6 loc=340
export namespace pbsd::port::wave6::hbsd::src::sys::netipsec::ipsec_mbuf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netipsec::ipsec_mbuf
