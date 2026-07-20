export module pbsd.port.wave9.hbsd.src.crypto.openssl.crypto.err.err_all_legacy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/crypto/err/err_all_legacy.c
// void err_all_legacy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/crypto/err/err_all_legacy.c wave=wave9 loc=106
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::err::err_all_legacy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::err::err_all_legacy
