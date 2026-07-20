export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.amd64.inout;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/amd64/inout.c
// void inout_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/amd64/inout.c wave=wave2 loc=295
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::amd64::inout {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::amd64::inout
