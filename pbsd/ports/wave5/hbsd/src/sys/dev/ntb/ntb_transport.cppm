export module pbsd.port.wave5.hbsd.src.sys.dev.ntb.ntb_transport;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ntb/ntb_transport.c
// void ntb_transport_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ntb/ntb_transport.c wave=wave5 loc=1693
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ntb::ntb_transport {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ntb::ntb_transport
