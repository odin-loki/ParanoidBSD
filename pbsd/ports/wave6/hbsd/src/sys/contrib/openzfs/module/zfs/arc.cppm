export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.zfs.arc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/zfs/arc.c
// void arc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/zfs/arc.c wave=wave6 loc=11313
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::arc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::zfs::arc
