export module pbsd.port.wave5.hbsd.src.sys.dev.vt.vt_font;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/vt/vt_font.c
// void vt_font_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/vt/vt_font.c wave=wave5 loc=219
export namespace pbsd::port::wave5::hbsd::src::sys::dev::vt::vt_font {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::vt::vt_font
