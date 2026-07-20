export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.amd64.e820;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/amd64/e820.c
// void e820_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/amd64/e820.c wave=wave2 loc=533
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::amd64::e820 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::amd64::e820
