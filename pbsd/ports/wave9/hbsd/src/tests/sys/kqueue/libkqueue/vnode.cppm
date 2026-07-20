export module pbsd.port.wave9.hbsd.src.tests.sys.kqueue.libkqueue.vnode;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/kqueue/libkqueue/vnode.c
// void vnode_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/kqueue/libkqueue/vnode.c wave=wave9 loc=322
export namespace pbsd::port::wave9::hbsd::src::tests::sys::kqueue::libkqueue::vnode {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::kqueue::libkqueue::vnode
