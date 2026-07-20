export module pbsd.port.wave5.hbsd.src.sys.dev.vt.font.vt_font_default;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/vt/font/vt_font_default.c
// void vt_font_default_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/vt/font/vt_font_default.c wave=wave5 loc=3495
export namespace pbsd::port::wave5::hbsd::src::sys::dev::vt::font::vt_font_default {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::vt::font::vt_font_default
