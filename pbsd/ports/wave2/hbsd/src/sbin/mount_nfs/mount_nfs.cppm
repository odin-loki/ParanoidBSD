export module pbsd.port.wave2.hbsd.src.sbin.mount_nfs.mount_nfs;

module;
extern "C" {
// Legacy TU entry — hand-port algorithm from hbsd/src/sbin/mount_nfs/mount_nfs.c
int main(int argc, char* argv[]);
}

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sbin/mount_nfs/mount_nfs.c wave=wave2 loc=1166
export namespace pbsd::port::wave2::hbsd::src::sbin::mount_nfs::mount_nfs {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::sbin::mount_nfs::mount_nfs
