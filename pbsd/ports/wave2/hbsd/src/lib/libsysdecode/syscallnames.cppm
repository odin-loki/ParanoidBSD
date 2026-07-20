export module pbsd.port.wave2.hbsd.src.lib.libsysdecode.syscallnames;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libsysdecode/syscallnames.c
// void syscallnames_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libsysdecode/syscallnames.c wave=wave2 loc=96
export namespace pbsd::port::wave2::hbsd::src::lib::libsysdecode::syscallnames {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libsysdecode::syscallnames
