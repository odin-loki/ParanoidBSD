export module pbsd.port.wave9.hbsd.src.crypto.openssl.test.cc_dummy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/test/cc_dummy.c
// void cc_dummy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/test/cc_dummy.c wave=wave9 loc=150
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::test::cc_dummy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::test::cc_dummy
