export module pbsd.port.wave4.hbsd.src.sys.kern.subr_coverage;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_coverage.c
// void subr_coverage_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_coverage.c wave=wave4 loc=236
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_coverage {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_coverage
