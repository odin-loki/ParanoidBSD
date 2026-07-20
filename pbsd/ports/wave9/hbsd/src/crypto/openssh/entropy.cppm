export module pbsd.port.wave9.hbsd.src.crypto.openssh.entropy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/entropy.c
// void entropy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/entropy.c wave=wave9 loc=109
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::entropy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::entropy
