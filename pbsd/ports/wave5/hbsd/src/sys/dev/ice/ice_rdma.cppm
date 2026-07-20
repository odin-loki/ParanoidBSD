export module pbsd.port.wave5.hbsd.src.sys.dev.ice.ice_rdma;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ice/ice_rdma.c
// void ice_rdma_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ice/ice_rdma.c wave=wave5 loc=932
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ice::ice_rdma {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ice::ice_rdma
