export module pbsd.port.wave6.hbsd.src.sys.fs.nfs.nfs_var;

module;
// Header bridge — replace #include of hbsd/src/sys/fs/nfs/nfs_var.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/fs/nfs/nfs_var.h wave=wave6 loc=817
export namespace pbsd::port::wave6::hbsd::src::sys::fs::nfs::nfs_var {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::fs::nfs::nfs_var
