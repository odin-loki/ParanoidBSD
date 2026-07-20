export module pbsd.port.wave9.hbsd.src.crypto.openssh.ssh_rsa;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/ssh-rsa.c
// void ssh-rsa_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/ssh-rsa.c wave=wave9 loc=645
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_rsa {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_rsa
