export module pbsd.port.wave5.hbsd.src.sys.dev.ipmi.ipmi_kcs;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ipmi/ipmi_kcs.c
// void ipmi_kcs_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ipmi/ipmi_kcs.c wave=wave5 loc=659
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ipmi::ipmi_kcs {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ipmi::ipmi_kcs
