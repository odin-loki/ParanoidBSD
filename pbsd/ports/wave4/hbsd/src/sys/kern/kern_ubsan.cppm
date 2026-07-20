export module pbsd.port.wave4.hbsd.src.sys.kern.kern_ubsan;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/kern_ubsan.c
// void kern_ubsan_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/kern_ubsan.c wave=wave4 loc=1703
export namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_ubsan {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_ubsan
