export module pbsd.port.wave2.hbsd.src.usr_sbin.traceroute.traceroute;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/traceroute/traceroute.c
// void traceroute_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/traceroute/traceroute.c wave=wave2 loc=2095
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::traceroute::traceroute {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::traceroute::traceroute
