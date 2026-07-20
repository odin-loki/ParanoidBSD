export module pbsd.port.wave4.hbsd.src.sys.libkern.bcopy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/libkern/bcopy.c
// void bcopy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/libkern/bcopy.c wave=wave4 loc=137
export namespace pbsd::port::wave4::hbsd::src::sys::libkern::bcopy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::libkern::bcopy
