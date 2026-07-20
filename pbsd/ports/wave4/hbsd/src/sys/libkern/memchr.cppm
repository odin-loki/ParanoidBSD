export module pbsd.port.wave4.hbsd.src.sys.libkern.memchr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/libkern/memchr.c
// void memchr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/libkern/memchr.c wave=wave4 loc=51
export namespace pbsd::port::wave4::hbsd::src::sys::libkern::memchr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::libkern::memchr
