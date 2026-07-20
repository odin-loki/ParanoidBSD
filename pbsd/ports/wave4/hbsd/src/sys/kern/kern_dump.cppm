export module pbsd.port.wave4.hbsd.src.sys.kern.kern_dump;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/kern_dump.c
// void kern_dump_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/kern_dump.c wave=wave4 loc=533
export namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_dump {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_dump
