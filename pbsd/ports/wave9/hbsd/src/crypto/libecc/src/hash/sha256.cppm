export module pbsd.port.wave9.hbsd.src.crypto.libecc.src.hash.sha256;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/libecc/src/hash/sha256.c
// void sha256_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/libecc/src/hash/sha256.c wave=wave9 loc=241
export namespace pbsd::port::wave9::hbsd::src::crypto::libecc::src::hash::sha256 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::libecc::src::hash::sha256
