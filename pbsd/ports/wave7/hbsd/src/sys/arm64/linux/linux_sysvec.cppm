export module pbsd.port.wave7.hbsd.src.sys.arm64.linux.linux_sysvec;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/arm64/linux/linux_sysvec.c
// void linux_sysvec_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/arm64/linux/linux_sysvec.c wave=wave7 loc=671
export namespace pbsd::port::wave7::hbsd::src::sys::arm64::linux::linux_sysvec {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::arm64::linux::linux_sysvec
