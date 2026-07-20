export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.bhyverun;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/bhyverun.c
// void bhyverun_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/bhyverun.c wave=wave2 loc=1022
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::bhyverun {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::bhyverun
