export module pbsd.port.wave5.hbsd.src.sys.dev.vkbd.vkbd;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/vkbd/vkbd.c
// void vkbd_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/vkbd/vkbd.c wave=wave5 loc=1377
export namespace pbsd::port::wave5::hbsd::src::sys::dev::vkbd::vkbd {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::vkbd::vkbd
