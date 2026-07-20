export module pbsd.port.wave5.hbsd.src.sys.dev.intpm.intpm;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/intpm/intpm.c
// void intpm_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/intpm/intpm.c wave=wave5 loc=923
export namespace pbsd::port::wave5::hbsd::src::sys::dev::intpm::intpm {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::intpm::intpm
