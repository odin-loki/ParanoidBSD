export module pbsd.port.wave5.hbsd.src.sys.dev.coretemp.coretemp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/coretemp/coretemp.c
// void coretemp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/coretemp/coretemp.c wave=wave5 loc=440
export namespace pbsd::port::wave5::hbsd::src::sys::dev::coretemp::coretemp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::coretemp::coretemp
