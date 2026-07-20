export module pbsd.port.wave7.hbsd.src.sys.x86.xen.xen_arch_intr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/x86/xen/xen_arch_intr.c
// void xen_arch_intr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/x86/xen/xen_arch_intr.c wave=wave7 loc=382
export namespace pbsd::port::wave7::hbsd::src::sys::x86::xen::xen_arch_intr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::x86::xen::xen_arch_intr
