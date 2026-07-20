export module pbsd.port.wave5.hbsd.src.sys.dev.ice.ice_vlan_mode;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ice/ice_vlan_mode.c
// void ice_vlan_mode_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ice/ice_vlan_mode.c wave=wave5 loc=325
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ice::ice_vlan_mode {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ice::ice_vlan_mode
