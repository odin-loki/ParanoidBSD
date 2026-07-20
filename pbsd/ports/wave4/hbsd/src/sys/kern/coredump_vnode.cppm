export module pbsd.port.wave4.hbsd.src.sys.kern.coredump_vnode;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/coredump_vnode.c
// void coredump_vnode_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/coredump_vnode.c wave=wave4 loc=562
export namespace pbsd::port::wave4::hbsd::src::sys::kern::coredump_vnode {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::coredump_vnode
