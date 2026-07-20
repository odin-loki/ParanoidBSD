export module pbsd.port.wave9.hbsd.src.crypto.openssh.openbsd_compat.libressl_api_compat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/openbsd-compat/libressl-api-compat.c
// void libressl-api-compat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/openbsd-compat/libressl-api-compat.c wave=wave9 loc=88
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::libressl_api_compat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::libressl_api_compat
