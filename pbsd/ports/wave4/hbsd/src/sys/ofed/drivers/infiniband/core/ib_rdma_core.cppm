export module pbsd.port.wave4.hbsd.src.sys.ofed.drivers.infiniband.core.ib_rdma_core;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/ofed/drivers/infiniband/core/ib_rdma_core.c
// void ib_rdma_core_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/ofed/drivers/infiniband/core/ib_rdma_core.c wave=wave4 loc=941
export namespace pbsd::port::wave4::hbsd::src::sys::ofed::drivers::infiniband::core::ib_rdma_core {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::ofed::drivers::infiniband::core::ib_rdma_core
