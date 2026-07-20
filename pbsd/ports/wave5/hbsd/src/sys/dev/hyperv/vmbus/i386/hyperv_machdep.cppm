export module pbsd.port.wave5.hbsd.src.sys.dev.hyperv.vmbus.i386.hyperv_machdep;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/hyperv/vmbus/i386/hyperv_machdep.c
// void hyperv_machdep_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/hyperv/vmbus/i386/hyperv_machdep.c wave=wave5 loc=48
export namespace pbsd::port::wave5::hbsd::src::sys::dev::hyperv::vmbus::i386::hyperv_machdep {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::hyperv::vmbus::i386::hyperv_machdep
