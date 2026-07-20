export module pbsd.port.wave5.hbsd.src.sys.dev.kbd.kbd;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/kbd/kbd.c
// void kbd_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/kbd/kbd.c wave=wave5 loc=1538
export namespace pbsd::port::wave5::hbsd::src::sys::dev::kbd::kbd {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::kbd::kbd
