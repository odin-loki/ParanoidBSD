export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.lib.libuutil.uu_alloc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/lib/libuutil/uu_alloc.c
// void uu_alloc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/lib/libuutil/uu_alloc.c wave=wave6 loc=136
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libuutil::uu_alloc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libuutil::uu_alloc
