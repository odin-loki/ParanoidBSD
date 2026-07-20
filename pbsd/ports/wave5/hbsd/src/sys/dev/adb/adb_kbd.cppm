export module pbsd.port.wave5.hbsd.src.sys.dev.adb.adb_kbd;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/adb/adb_kbd.c
// void adb_kbd_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/adb/adb_kbd.c wave=wave5 loc=881
export namespace pbsd::port::wave5::hbsd::src::sys::dev::adb::adb_kbd {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::adb::adb_kbd
