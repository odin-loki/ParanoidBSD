export module pbsd.port.wave9.hbsd.src.kerberos5.lib.libgssapi_spnego.freebsd_compat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/kerberos5/lib/libgssapi_spnego/freebsd_compat.c
// void freebsd_compat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/kerberos5/lib/libgssapi_spnego/freebsd_compat.c wave=wave9 loc=84
export namespace pbsd::port::wave9::hbsd::src::kerberos5::lib::libgssapi_spnego::freebsd_compat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::kerberos5::lib::libgssapi_spnego::freebsd_compat
