export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.space_map;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/space_map.c
// void space_map_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/space_map.c wave=wave6 loc=1112
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::space_map {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::space_map
