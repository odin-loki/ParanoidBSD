export module pbsd.port.wave9.hbsd.src.crypto.openssl.apps.genpkey;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/apps/genpkey.c
// void genpkey_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/apps/genpkey.c wave=wave9 loc=425
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::apps::genpkey {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::apps::genpkey
