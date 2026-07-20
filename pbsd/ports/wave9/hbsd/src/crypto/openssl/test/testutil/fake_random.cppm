export module pbsd.port.wave9.hbsd.src.crypto.openssl.test.testutil.fake_random;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/test/testutil/fake_random.c
// void fake_random_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/test/testutil/fake_random.c wave=wave9 loc=230
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::test::testutil::fake_random {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::test::testutil::fake_random
