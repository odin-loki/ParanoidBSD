export module pbsd.port.wave9.hbsd.src.crypto.openssh.auth2_none;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/auth2-none.c
// void auth2-none_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/auth2-none.c wave=wave9 loc=76
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::auth2_none {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::auth2_none
