export module pbsd.port.wave2.hbsd.src.lib.libprocstat.libprocstat_compat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libprocstat/libprocstat_compat.c
// void libprocstat_compat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libprocstat/libprocstat_compat.c wave=wave2 loc=259
export namespace pbsd::port::wave2::hbsd::src::lib::libprocstat::libprocstat_compat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libprocstat::libprocstat_compat
