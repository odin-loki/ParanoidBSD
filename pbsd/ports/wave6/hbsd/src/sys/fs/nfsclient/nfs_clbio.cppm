export module pbsd.port.wave6.hbsd.src.sys.fs.nfsclient.nfs_clbio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/fs/nfsclient/nfs_clbio.c
// void nfs_clbio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/fs/nfsclient/nfs_clbio.c wave=wave6 loc=1841
export namespace pbsd::port::wave6::hbsd::src::sys::fs::nfsclient::nfs_clbio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::fs::nfsclient::nfs_clbio
