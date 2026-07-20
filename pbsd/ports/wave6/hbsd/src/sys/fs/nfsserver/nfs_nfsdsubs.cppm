export module pbsd.port.wave6.hbsd.src.sys.fs.nfsserver.nfs_nfsdsubs;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/fs/nfsserver/nfs_nfsdsubs.c
// void nfs_nfsdsubs_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/fs/nfsserver/nfs_nfsdsubs.c wave=wave6 loc=2268
export namespace pbsd::port::wave6::hbsd::src::sys::fs::nfsserver::nfs_nfsdsubs {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::fs::nfsserver::nfs_nfsdsubs
