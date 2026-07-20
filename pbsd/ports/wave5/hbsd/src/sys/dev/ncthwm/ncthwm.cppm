export module pbsd.port.wave5.hbsd.src.sys.dev.ncthwm.ncthwm;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ncthwm/ncthwm.c
// void ncthwm_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ncthwm/ncthwm.c wave=wave5 loc=258
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ncthwm::ncthwm {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ncthwm::ncthwm
