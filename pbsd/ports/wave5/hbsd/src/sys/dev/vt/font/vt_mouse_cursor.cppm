export module pbsd.port.wave5.hbsd.src.sys.dev.vt.font.vt_mouse_cursor;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/vt/font/vt_mouse_cursor.c
// void vt_mouse_cursor_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/vt/font/vt_mouse_cursor.c wave=wave5 loc=75
export namespace pbsd::port::wave5::hbsd::src::sys::dev::vt::font::vt_mouse_cursor {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::vt::font::vt_mouse_cursor
