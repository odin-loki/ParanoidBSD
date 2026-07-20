export module pbsd.port.wave6.hbsd.src.sys.fs.nfsclient.nfs_clsubs;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/fs/nfsclient/nfs_clsubs.c
// void nfs_clsubs_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/fs/nfsclient/nfs_clsubs.c wave=wave6 loc=391
export namespace pbsd::port::wave6::hbsd::src::sys::fs::nfsclient::nfs_clsubs {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::fs::nfsclient::nfs_clsubs
