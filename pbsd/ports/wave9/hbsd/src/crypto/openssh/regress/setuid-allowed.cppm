export module pbsd.port.wave9.hbsd.src.crypto.openssh.regress.setuid_allowed;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/regress/setuid-allowed.c
// void setuid-allowed_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/regress/setuid-allowed.c wave=wave9 loc=58
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::regress::setuid_allowed {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::regress::setuid_allowed
