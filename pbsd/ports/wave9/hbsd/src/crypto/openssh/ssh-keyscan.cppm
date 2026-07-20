export module pbsd.port.wave9.hbsd.src.crypto.openssh.ssh_keyscan;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/ssh-keyscan.c
// void ssh-keyscan_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/ssh-keyscan.c wave=wave9 loc=848
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_keyscan {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_keyscan
