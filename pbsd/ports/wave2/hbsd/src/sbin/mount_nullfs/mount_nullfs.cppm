export module pbsd.port.wave2.hbsd.src.sbin.mount_nullfs.mount_nullfs;

module;
extern "C" {
// Legacy TU entry — hand-port algorithm from hbsd/src/sbin/mount_nullfs/mount_nullfs.c
int main(int argc, char* argv[]);
}

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sbin/mount_nullfs/mount_nullfs.c wave=wave2 loc=129
export namespace pbsd::port::wave2::hbsd::src::sbin::mount_nullfs::mount_nullfs {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::sbin::mount_nullfs::mount_nullfs
