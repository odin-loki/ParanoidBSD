export module pbsd.port.wave9.hbsd.src.crypto.openssh.kex_names;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/kex-names.c
// void kex-names_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/kex-names.c wave=wave9 loc=336
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::kex_names {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::kex_names
