export module pbsd.port.wave4.hbsd.src.sys.libkern.timingsafe_bcmp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/libkern/timingsafe_bcmp.c
// void timingsafe_bcmp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/libkern/timingsafe_bcmp.c wave=wave4 loc=30
export namespace pbsd::port::wave4::hbsd::src::sys::libkern::timingsafe_bcmp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::libkern::timingsafe_bcmp
