export module pbsd.port.wave5.hbsd.src.sys.dev.syscons.scmouse;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/syscons/scmouse.c
// void scmouse_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/syscons/scmouse.c wave=wave5 loc=958
export namespace pbsd::port::wave5::hbsd::src::sys::dev::syscons::scmouse {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::syscons::scmouse
