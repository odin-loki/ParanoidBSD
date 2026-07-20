export module pbsd.port.wave9.hbsd.src.crypto.openssh.ssh_ecdsa_sk;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/ssh-ecdsa-sk.c
// void ssh-ecdsa-sk_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/ssh-ecdsa-sk.c wave=wave9 loc=492
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_ecdsa_sk {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_ecdsa_sk
