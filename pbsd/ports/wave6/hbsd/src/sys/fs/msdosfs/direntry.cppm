export module pbsd.port.wave6.hbsd.src.sys.fs.msdosfs.direntry;

module;
// Header bridge — replace #include of hbsd/src/sys/fs/msdosfs/direntry.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/fs/msdosfs/direntry.h wave=wave6 loc=165
export namespace pbsd::port::wave6::hbsd::src::sys::fs::msdosfs::direntry {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::fs::msdosfs::direntry
