export module pbsd.port.wave9.hbsd.src.crypto.openssl.test.testutil.random;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/test/testutil/random.c
// void random_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/test/testutil/random.c wave=wave9 loc=42
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::test::testutil::random {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::test::testutil::random
