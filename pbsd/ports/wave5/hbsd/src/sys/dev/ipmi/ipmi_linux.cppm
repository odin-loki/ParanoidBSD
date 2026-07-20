export module pbsd.port.wave5.hbsd.src.sys.dev.ipmi.ipmi_linux;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ipmi/ipmi_linux.c
// void ipmi_linux_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ipmi/ipmi_linux.c wave=wave5 loc=109
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ipmi::ipmi_linux {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ipmi::ipmi_linux
