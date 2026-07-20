export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.lib.libspl.list;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/lib/libspl/list.c
// void list_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/lib/libspl/list.c wave=wave6 loc=244
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libspl::list {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libspl::list
