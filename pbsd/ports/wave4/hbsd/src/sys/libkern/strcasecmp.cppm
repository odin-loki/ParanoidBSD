export module pbsd.port.wave4.hbsd.src.sys.libkern.strcasecmp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/libkern/strcasecmp.c
// void strcasecmp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/libkern/strcasecmp.c wave=wave4 loc=70
export namespace pbsd::port::wave4::hbsd::src::sys::libkern::strcasecmp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::libkern::strcasecmp
