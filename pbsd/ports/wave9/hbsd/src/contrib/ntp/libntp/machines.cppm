export module pbsd.port.wave9.hbsd.src.contrib.ntp.libntp.machines;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/libntp/machines.c
// void machines_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/libntp/machines.c wave=wave9 loc=587
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::libntp::machines {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::libntp::machines
