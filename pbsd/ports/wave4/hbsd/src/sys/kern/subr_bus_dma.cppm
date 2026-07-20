export module pbsd.port.wave4.hbsd.src.sys.kern.subr_bus_dma;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_bus_dma.c
// void subr_bus_dma_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_bus_dma.c wave=wave4 loc=702
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_bus_dma {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_bus_dma
