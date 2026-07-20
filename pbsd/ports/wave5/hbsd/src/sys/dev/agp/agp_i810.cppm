export module pbsd.port.wave5.hbsd.src.sys.dev.agp.agp_i810;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/agp/agp_i810.c
// void agp_i810_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/agp/agp_i810.c wave=wave5 loc=2372
export namespace pbsd::port::wave5::hbsd::src::sys::dev::agp::agp_i810 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::agp::agp_i810
