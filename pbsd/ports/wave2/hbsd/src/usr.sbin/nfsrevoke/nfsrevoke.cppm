export module pbsd.port.wave2.hbsd.src.usr_sbin.nfsrevoke.nfsrevoke;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/nfsrevoke/nfsrevoke.c
// void nfsrevoke_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/nfsrevoke/nfsrevoke.c wave=wave2 loc=120
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::nfsrevoke::nfsrevoke {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::nfsrevoke::nfsrevoke
