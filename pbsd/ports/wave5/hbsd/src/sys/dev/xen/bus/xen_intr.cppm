export module pbsd.port.wave5.hbsd.src.sys.dev.xen.bus.xen_intr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/xen/bus/xen_intr.c
// void xen_intr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/xen/bus/xen_intr.c wave=wave5 loc=1148
export namespace pbsd::port::wave5::hbsd::src::sys::dev::xen::bus::xen_intr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::xen::bus::xen_intr
