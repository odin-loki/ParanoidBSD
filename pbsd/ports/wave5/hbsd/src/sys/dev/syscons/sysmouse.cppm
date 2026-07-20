export module pbsd.port.wave5.hbsd.src.sys.dev.syscons.sysmouse;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/syscons/sysmouse.c
// void sysmouse_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/syscons/sysmouse.c wave=wave5 loc=338
export namespace pbsd::port::wave5::hbsd::src::sys::dev::syscons::sysmouse {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::syscons::sysmouse
