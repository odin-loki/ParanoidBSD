export module pbsd.port.wave5.hbsd.src.sys.dev.vt.hw.vga.vt_vga;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/vt/hw/vga/vt_vga.c
// void vt_vga_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/vt/hw/vga/vt_vga.c wave=wave5 loc=1402
export namespace pbsd::port::wave5::hbsd::src::sys::dev::vt::hw::vga::vt_vga {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::vt::hw::vga::vt_vga
