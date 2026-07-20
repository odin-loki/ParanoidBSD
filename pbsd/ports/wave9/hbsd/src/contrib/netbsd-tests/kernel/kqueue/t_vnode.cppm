export module pbsd.port.wave9.hbsd.src.contrib.netbsd_tests.kernel.kqueue.t_vnode;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/netbsd-tests/kernel/kqueue/t_vnode.c
// void t_vnode_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/netbsd-tests/kernel/kqueue/t_vnode.c wave=wave9 loc=533
export namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::kernel::kqueue::t_vnode {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::kernel::kqueue::t_vnode
