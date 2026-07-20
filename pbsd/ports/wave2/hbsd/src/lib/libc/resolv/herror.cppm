export module pbsd.port.wave2.hbsd.src.lib.libc.resolv.herror;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/resolv/herror.c
// void herror_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/resolv/herror.c wave=wave2 loc=120
export namespace pbsd::port::wave2::hbsd::src::lib::libc::resolv::herror {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::resolv::herror
