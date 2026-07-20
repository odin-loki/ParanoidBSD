export module pbsd.port.wave7.hbsd.src.sys.amd64.linux32.linux32_machdep;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/amd64/linux32/linux32_machdep.c
// void linux32_machdep_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/amd64/linux32/linux32_machdep.c wave=wave7 loc=614
export namespace pbsd::port::wave7::hbsd::src::sys::amd64::linux32::linux32_machdep {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::amd64::linux32::linux32_machdep
