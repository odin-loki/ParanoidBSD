export module pbsd.port.wave7.hbsd.src.sys.amd64.include.xen.arch_intr;

module;
// Header bridge — replace #include of hbsd/src/sys/amd64/include/xen/arch-intr.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/amd64/include/xen/arch-intr.h wave=wave7 loc=3
export namespace pbsd::port::wave7::hbsd::src::sys::amd64::include::xen::arch_intr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::amd64::include::xen::arch_intr
