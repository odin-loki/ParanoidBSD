export module pbsd.port.wave5.hbsd.src.sys.dev.agp.agp_amd64;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/agp/agp_amd64.c
// void agp_amd64_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/agp/agp_amd64.c wave=wave5 loc=512
export namespace pbsd::port::wave5::hbsd::src::sys::dev::agp::agp_amd64 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::agp::agp_amd64
