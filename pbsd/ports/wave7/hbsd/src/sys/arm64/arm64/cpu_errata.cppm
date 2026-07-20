export module pbsd.port.wave7.hbsd.src.sys.arm64.arm64.cpu_errata;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/arm64/arm64/cpu_errata.c
// void cpu_errata_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/arm64/arm64/cpu_errata.c wave=wave7 loc=145
export namespace pbsd::port::wave7::hbsd::src::sys::arm64::arm64::cpu_errata {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::arm64::arm64::cpu_errata
