export module pbsd.port.wave7.hbsd.src.sys.arm64.arm64.ofw_machdep;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/arm64/arm64/ofw_machdep.c
// void ofw_machdep_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/arm64/arm64/ofw_machdep.c wave=wave7 loc=61
export namespace pbsd::port::wave7::hbsd::src::sys::arm64::arm64::ofw_machdep {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::arm64::arm64::ofw_machdep
