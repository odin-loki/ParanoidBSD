export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.lib.libzutil.zutil_device_path;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/lib/libzutil/zutil_device_path.c
// void zutil_device_path_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/lib/libzutil/zutil_device_path.c wave=wave6 loc=205
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libzutil::zutil_device_path {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libzutil::zutil_device_path
