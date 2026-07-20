export module pbsd.port.wave2.hbsd.src.lib.libc.net.gai_strerror;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/net/gai_strerror.c
// void gai_strerror_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/net/gai_strerror.c wave=wave2 loc=124
export namespace pbsd::port::wave2::hbsd::src::lib::libc::net::gai_strerror {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::net::gai_strerror
