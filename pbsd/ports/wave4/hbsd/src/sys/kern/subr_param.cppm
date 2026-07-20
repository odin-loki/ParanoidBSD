export module pbsd.port.wave4.hbsd.src.sys.kern.subr_param;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_param.c
// void subr_param_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_param.c wave=wave4 loc=378
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_param {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_param
