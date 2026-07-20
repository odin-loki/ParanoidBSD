export module pbsd.port.wave2.hbsd.src.usr_sbin.iovctl.iovctl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/iovctl/iovctl.c
// void iovctl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/iovctl/iovctl.c wave=wave2 loc=410
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::iovctl::iovctl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::iovctl::iovctl
