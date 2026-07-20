export module pbsd.port.wave6.hbsd.src.sys.netipsec.ipsec_offload;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netipsec/ipsec_offload.c
// void ipsec_offload_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netipsec/ipsec_offload.c wave=wave6 loc=1211
export namespace pbsd::port::wave6::hbsd::src::sys::netipsec::ipsec_offload {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netipsec::ipsec_offload
