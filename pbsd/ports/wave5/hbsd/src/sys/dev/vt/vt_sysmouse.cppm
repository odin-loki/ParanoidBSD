export module pbsd.port.wave5.hbsd.src.sys.dev.vt.vt_sysmouse;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/vt/vt_sysmouse.c
// void vt_sysmouse_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/vt/vt_sysmouse.c wave=wave5 loc=500
export namespace pbsd::port::wave5::hbsd::src::sys::dev::vt::vt_sysmouse {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::vt::vt_sysmouse
