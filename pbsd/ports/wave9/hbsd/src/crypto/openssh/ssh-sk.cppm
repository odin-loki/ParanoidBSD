export module pbsd.port.wave9.hbsd.src.crypto.openssh.ssh_sk;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/ssh-sk.c
// void ssh-sk_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/ssh-sk.c wave=wave9 loc=894
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_sk {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_sk
