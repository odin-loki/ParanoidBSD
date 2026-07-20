export module pbsd.port.wave9.hbsd.src.crypto.openssh.auth2_gss;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/auth2-gss.c
// void auth2-gss_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/auth2-gss.c wave=wave9 loc=332
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::auth2_gss {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::auth2_gss
