export module pbsd.port.wave4.hbsd.src.sys.libkern.qsort_r;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/libkern/qsort_r.c
// void qsort_r_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/libkern/qsort_r.c wave=wave4 loc=6
export namespace pbsd::port::wave4::hbsd::src::sys::libkern::qsort_r {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::libkern::qsort_r
