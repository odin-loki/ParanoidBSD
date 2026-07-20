export module pbsd.port.wave2.hbsd.src.lib.libc.sys.getdents;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/sys/getdents.c
// void getdents_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/sys/getdents.c wave=wave2 loc=38
export namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::getdents {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::getdents
