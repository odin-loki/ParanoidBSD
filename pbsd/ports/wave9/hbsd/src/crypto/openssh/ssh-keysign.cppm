export module pbsd.port.wave9.hbsd.src.crypto.openssh.ssh_keysign;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/ssh-keysign.c
// void ssh-keysign_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/ssh-keysign.c wave=wave9 loc=311
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_keysign {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_keysign
