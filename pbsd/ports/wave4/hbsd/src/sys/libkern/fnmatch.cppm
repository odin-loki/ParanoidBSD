export module pbsd.port.wave4.hbsd.src.sys.libkern.fnmatch;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/libkern/fnmatch.c
// void fnmatch_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/libkern/fnmatch.c wave=wave4 loc=213
export namespace pbsd::port::wave4::hbsd::src::sys::libkern::fnmatch {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::libkern::fnmatch
