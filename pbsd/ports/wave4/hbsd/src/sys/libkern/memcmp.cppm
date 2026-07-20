export module pbsd.port.wave4.hbsd.src.sys.libkern.memcmp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/libkern/memcmp.c
// void memcmp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/libkern/memcmp.c wave=wave4 loc=53
export namespace pbsd::port::wave4::hbsd::src::sys::libkern::memcmp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::libkern::memcmp
