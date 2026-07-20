export module pbsd.port.wave5.hbsd.src.sys.dev.hyperv.vmbus.hyperv_busdma;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/hyperv/vmbus/hyperv_busdma.c
// void hyperv_busdma_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/hyperv/vmbus/hyperv_busdma.c wave=wave5 loc=48
export namespace pbsd::port::wave5::hbsd::src::sys::dev::hyperv::vmbus::hyperv_busdma {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::hyperv::vmbus::hyperv_busdma
