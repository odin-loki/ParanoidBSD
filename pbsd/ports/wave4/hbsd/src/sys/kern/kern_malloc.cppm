export module pbsd.port.wave4.hbsd.src.sys.kern.kern_malloc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/kern_malloc.c
// void kern_malloc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/kern_malloc.c wave=wave4 loc=1686
export namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_malloc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_malloc
