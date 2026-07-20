export module pbsd.port.wave5.hbsd.src.sys.dev.evdev.uinput;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/evdev/uinput.c
// void uinput_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/evdev/uinput.c wave=wave5 loc=715
export namespace pbsd::port::wave5::hbsd::src::sys::dev::evdev::uinput {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::evdev::uinput
