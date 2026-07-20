export module pbsd.port.wave5.hbsd.src.sys.dev.ipmi.ipmi_bt;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ipmi/ipmi_bt.c
// void ipmi_bt_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ipmi/ipmi_bt.c wave=wave5 loc=293
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ipmi::ipmi_bt {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ipmi::ipmi_bt
