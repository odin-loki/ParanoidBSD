export module pbsd.port.wave9.hbsd.src.crypto.openssh.ssh_sk_helper;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/ssh-sk-helper.c
// void ssh-sk-helper_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/ssh-sk-helper.c wave=wave9 loc=367
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_sk_helper {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_sk_helper
