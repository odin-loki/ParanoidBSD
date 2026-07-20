export module pbsd.port.wave5.hbsd.src.sys.dev.isp.isp_target;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/isp/isp_target.c
// void isp_target_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/isp/isp_target.c wave=wave5 loc=871
export namespace pbsd::port::wave5::hbsd::src::sys::dev::isp::isp_target {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::isp::isp_target
