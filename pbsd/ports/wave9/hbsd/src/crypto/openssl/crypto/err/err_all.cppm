export module pbsd.port.wave9.hbsd.src.crypto.openssl.crypto.err.err_all;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/crypto/err/err_all.c
// void err_all_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/crypto/err/err_all.c wave=wave9 loc=119
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::err::err_all {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::err::err_all
