export module pbsd.port.wave9.hbsd.src.crypto.openssh.ssh_pkcs11_client;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/ssh-pkcs11-client.c
// void ssh-pkcs11-client_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/ssh-pkcs11-client.c wave=wave9 loc=697
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_pkcs11_client {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_pkcs11_client
