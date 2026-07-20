export module pbsd.port.wave4.hbsd.src.sys.libkern.strcpy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/libkern/strcpy.c
// void strcpy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/libkern/strcpy.c wave=wave4 loc=42
export namespace pbsd::port::wave4::hbsd::src::sys::libkern::strcpy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::libkern::strcpy
