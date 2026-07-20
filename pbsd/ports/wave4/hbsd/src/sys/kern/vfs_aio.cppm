export module pbsd.port.wave4.hbsd.src.sys.kern.vfs_aio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/vfs_aio.c
// void vfs_aio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/vfs_aio.c wave=wave4 loc=3174
export namespace pbsd::port::wave4::hbsd::src::sys::kern::vfs_aio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::vfs_aio
