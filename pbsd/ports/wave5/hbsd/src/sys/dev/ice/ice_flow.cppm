export module pbsd.port.wave5.hbsd.src.sys.dev.ice.ice_flow;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ice/ice_flow.c
// void ice_flow_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ice/ice_flow.c wave=wave5 loc=1972
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ice::ice_flow {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ice::ice_flow
