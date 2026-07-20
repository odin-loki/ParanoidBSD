export module pbsd.port.wave9.hbsd.src.crypto.openssl.providers.implementations.rands.seeding.rand_unix;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/providers/implementations/rands/seeding/rand_unix.c
// void rand_unix_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/providers/implementations/rands/seeding/rand_unix.c wave=wave9 loc=803
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::providers::implementations::rands::seeding::rand_unix {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::providers::implementations::rands::seeding::rand_unix
