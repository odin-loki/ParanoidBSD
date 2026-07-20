export module pbsd.port.wave9.hbsd.src.crypto.openssh.sandbox_null;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/sandbox-null.c
// void sandbox-null_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/sandbox-null.c wave=wave9 loc=60
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::sandbox_null {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::sandbox_null
