export module pbsd.port.wave4.hbsd.src.sys.kern.vfs_bio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/vfs_bio.c
// void vfs_bio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/vfs_bio.c wave=wave4 loc=5693
export namespace pbsd::port::wave4::hbsd::src::sys::kern::vfs_bio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::vfs_bio
