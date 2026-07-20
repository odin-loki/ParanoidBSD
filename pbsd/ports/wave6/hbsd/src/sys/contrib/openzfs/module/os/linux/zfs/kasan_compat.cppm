export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.os.linux.zfs.kasan_compat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/os/linux/zfs/kasan_compat.c
// void kasan_compat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/os/linux/zfs/kasan_compat.c wave=wave6 loc=48
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::zfs::kasan_compat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::os::linux::zfs::kasan_compat
