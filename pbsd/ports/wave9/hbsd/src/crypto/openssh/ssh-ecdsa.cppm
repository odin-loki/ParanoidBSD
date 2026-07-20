export module pbsd.port.wave9.hbsd.src.crypto.openssh.ssh_ecdsa;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/ssh-ecdsa.c
// void ssh-ecdsa_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/ssh-ecdsa.c wave=wave9 loc=564
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_ecdsa {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_ecdsa
