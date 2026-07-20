export module pbsd.port.wave7.hbsd.src.sys.amd64.amd64.gdb_machdep;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/amd64/amd64/gdb_machdep.c
// void gdb_machdep_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/amd64/amd64/gdb_machdep.c wave=wave7 loc=199
export namespace pbsd::port::wave7::hbsd::src::sys::amd64::amd64::gdb_machdep {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::amd64::amd64::gdb_machdep
