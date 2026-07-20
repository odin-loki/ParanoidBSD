export module pbsd.port.wave5.hbsd.src.sys.dev.ipmi.ipmi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ipmi/ipmi.c
// void ipmi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ipmi/ipmi.c wave=wave5 loc=1117
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ipmi::ipmi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ipmi::ipmi
