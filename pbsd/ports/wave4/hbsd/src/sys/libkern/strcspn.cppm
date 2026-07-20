export module pbsd.port.wave4.hbsd.src.sys.libkern.strcspn;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/libkern/strcspn.c
// void strcspn_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/libkern/strcspn.c wave=wave4 loc=72
export namespace pbsd::port::wave4::hbsd::src::sys::libkern::strcspn {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::libkern::strcspn
