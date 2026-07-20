export module pbsd.port.wave9.hbsd.src.crypto.openssh.kex;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/kex.c
// void kex_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/kex.c wave=wave9 loc=1438
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::kex {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::kex
