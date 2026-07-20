export module pbsd.port.wave9.hbsd.src.crypto.openssh.ssh_ed25519;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/ssh-ed25519.c
// void ssh-ed25519_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/ssh-ed25519.c wave=wave9 loc=313
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_ed25519 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_ed25519
