export module pbsd.port.wave5.hbsd.src.sys.dev.ipmi.ipmi_smbus;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ipmi/ipmi_smbus.c
// void ipmi_smbus_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ipmi/ipmi_smbus.c wave=wave5 loc=134
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ipmi::ipmi_smbus {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ipmi::ipmi_smbus
