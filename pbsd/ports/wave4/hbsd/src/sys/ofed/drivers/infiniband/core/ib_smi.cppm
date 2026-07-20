export module pbsd.port.wave4.hbsd.src.sys.ofed.drivers.infiniband.core.ib_smi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/ofed/drivers/infiniband/core/ib_smi.c
// void ib_smi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/ofed/drivers/infiniband/core/ib_smi.c wave=wave4 loc=340
export namespace pbsd::port::wave4::hbsd::src::sys::ofed::drivers::infiniband::core::ib_smi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::ofed::drivers::infiniband::core::ib_smi
