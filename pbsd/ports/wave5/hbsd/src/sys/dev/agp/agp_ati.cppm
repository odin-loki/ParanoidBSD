export module pbsd.port.wave5.hbsd.src.sys.dev.agp.agp_ati;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/agp/agp_ati.c
// void agp_ati_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/agp/agp_ati.c wave=wave5 loc=376
export namespace pbsd::port::wave5::hbsd::src::sys::dev::agp::agp_ati {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::agp::agp_ati
