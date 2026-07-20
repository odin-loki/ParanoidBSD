export module pbsd.port.wave2.hbsd.src.usr_sbin.nologin.nologin;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/nologin/nologin.c
// void nologin_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/nologin/nologin.c wave=wave2 loc=49
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::nologin::nologin {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::nologin::nologin
