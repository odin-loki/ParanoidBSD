export module pbsd.port.wave5.hbsd.src.sys.dev.isp.isp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/isp/isp.c
// void isp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/isp/isp.c wave=wave5 loc=5416
export namespace pbsd::port::wave5::hbsd::src::sys::dev::isp::isp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::isp::isp
