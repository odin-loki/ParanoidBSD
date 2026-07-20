export module pbsd.port.wave6.hbsd.src.sys.fs.fuse.fuse_file;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/fs/fuse/fuse_file.c
// void fuse_file_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/fs/fuse/fuse_file.c wave=wave6 loc=405
export namespace pbsd::port::wave6::hbsd::src::sys::fs::fuse::fuse_file {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::fs::fuse::fuse_file
