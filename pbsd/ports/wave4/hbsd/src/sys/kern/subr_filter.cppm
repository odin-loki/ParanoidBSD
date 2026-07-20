export module pbsd.port.wave4.hbsd.src.sys.kern.subr_filter;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_filter.c
// void subr_filter_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_filter.c wave=wave4 loc=473
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_filter {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_filter
