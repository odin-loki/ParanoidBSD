export module pbsd.port.wave2.hbsd.src.usr_sbin.fstyp.ntfs;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/fstyp/ntfs.c
// void ntfs_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/fstyp/ntfs.c wave=wave2 loc=210
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::fstyp::ntfs {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::fstyp::ntfs
