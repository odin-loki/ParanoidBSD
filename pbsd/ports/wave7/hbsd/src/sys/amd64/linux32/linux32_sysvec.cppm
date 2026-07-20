export module pbsd.port.wave7.hbsd.src.sys.amd64.linux32.linux32_sysvec;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/amd64/linux32/linux32_sysvec.c
// void linux32_sysvec_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/amd64/linux32/linux32_sysvec.c wave=wave7 loc=1068
export namespace pbsd::port::wave7::hbsd::src::sys::amd64::linux32::linux32_sysvec {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::amd64::linux32::linux32_sysvec
