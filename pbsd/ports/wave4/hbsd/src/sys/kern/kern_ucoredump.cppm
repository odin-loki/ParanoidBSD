export module pbsd.port.wave4.hbsd.src.sys.kern.kern_ucoredump;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/kern_ucoredump.c
// void kern_ucoredump_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/kern_ucoredump.c wave=wave4 loc=308
export namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_ucoredump {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_ucoredump
