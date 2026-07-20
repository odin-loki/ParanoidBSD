export module pbsd.port.wave2.hbsd.src.lib.libc.stdlib.ptsname;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdlib/ptsname.c
// void ptsname_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdlib/ptsname.c wave=wave2 loc=114
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::ptsname {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::ptsname
