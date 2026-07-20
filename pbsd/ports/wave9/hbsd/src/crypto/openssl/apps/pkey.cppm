export module pbsd.port.wave9.hbsd.src.crypto.openssl.apps.pkey;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/apps/pkey.c
// void pkey_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/apps/pkey.c wave=wave9 loc=344
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::apps::pkey {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::apps::pkey
