export module pbsd.port.wave2.hbsd.src.cddl.usr_sbin.zfsd.zpool_list;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/cddl/usr.sbin/zfsd/zpool_list.cc
// void zpool_list_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/cddl/usr.sbin/zfsd/zpool_list.cc wave=wave2 loc=120
export namespace pbsd::port::wave2::hbsd::src::cddl::usr_sbin::zfsd::zpool_list {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::cddl::usr_sbin::zfsd::zpool_list
