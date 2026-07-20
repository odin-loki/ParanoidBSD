export module pbsd.port.wave6.hbsd.src.sys.netipsec.xform_tcp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netipsec/xform_tcp.c
// void xform_tcp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netipsec/xform_tcp.c wave=wave6 loc=439
export namespace pbsd::port::wave6::hbsd::src::sys::netipsec::xform_tcp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netipsec::xform_tcp
