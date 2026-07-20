export module pbsd.port.wave9.hbsd.src.crypto.openssh.ssh_ed25519_sk;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/ssh-ed25519-sk.c
// void ssh-ed25519-sk_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/ssh-ed25519-sk.c wave=wave9 loc=288
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_ed25519_sk {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_ed25519_sk
