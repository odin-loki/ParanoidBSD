export module pbsd.port.wave6.hbsd.src.sys.fs.fuse.fuse_node;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/fs/fuse/fuse_node.c
// void fuse_node_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/fs/fuse/fuse_node.c wave=wave6 loc=609
export namespace pbsd::port::wave6::hbsd::src::sys::fs::fuse::fuse_node {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::fs::fuse::fuse_node
