export module pbsd.port.wave5.hbsd.src.sys.dev.adb.adb_mouse;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/adb/adb_mouse.c
// void adb_mouse_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/adb/adb_mouse.c wave=wave5 loc=679
export namespace pbsd::port::wave5::hbsd::src::sys::dev::adb::adb_mouse {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::adb::adb_mouse
