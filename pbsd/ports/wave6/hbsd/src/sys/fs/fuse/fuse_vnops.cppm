export module pbsd.port.wave6.hbsd.src.sys.fs.fuse.fuse_vnops;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/fs/fuse/fuse_vnops.c
// void fuse_vnops_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/fs/fuse/fuse_vnops.c wave=wave6 loc=3364
export namespace pbsd::port::wave6::hbsd::src::sys::fs::fuse::fuse_vnops {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::fs::fuse::fuse_vnops
