export module pbsd.port.wave2.hbsd.src.usr_sbin.ypserv.yp_error;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/ypserv/yp_error.c
// void yp_error_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/ypserv/yp_error.c wave=wave2 loc=71
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::ypserv::yp_error {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::ypserv::yp_error
