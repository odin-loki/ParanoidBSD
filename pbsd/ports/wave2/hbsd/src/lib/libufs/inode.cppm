export module pbsd.port.wave2.hbsd.src.lib.libufs.inode;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libufs/inode.c
// void inode_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libufs/inode.c wave=wave2 loc=123
export namespace pbsd::port::wave2::hbsd::src::lib::libufs::inode {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libufs::inode
