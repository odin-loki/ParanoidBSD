export module pbsd.port.wave9.hbsd.src.contrib.ntp.libntp.libssl_compat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/libntp/libssl_compat.c
// void libssl_compat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/libntp/libssl_compat.c wave=wave9 loc=341
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::libntp::libssl_compat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::libntp::libssl_compat
