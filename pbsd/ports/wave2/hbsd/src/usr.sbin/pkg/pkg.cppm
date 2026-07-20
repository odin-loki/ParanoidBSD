export module pbsd.port.wave2.hbsd.src.usr_sbin.pkg.pkg;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/pkg/pkg.c
// void pkg_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/pkg/pkg.c wave=wave2 loc=1264
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::pkg::pkg {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::pkg::pkg
