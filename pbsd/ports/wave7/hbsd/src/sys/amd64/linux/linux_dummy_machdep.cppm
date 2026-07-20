export module pbsd.port.wave7.hbsd.src.sys.amd64.linux.linux_dummy_machdep;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/amd64/linux/linux_dummy_machdep.c
// void linux_dummy_machdep_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/amd64/linux/linux_dummy_machdep.c wave=wave7 loc=64
export namespace pbsd::port::wave7::hbsd::src::sys::amd64::linux::linux_dummy_machdep {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::amd64::linux::linux_dummy_machdep
