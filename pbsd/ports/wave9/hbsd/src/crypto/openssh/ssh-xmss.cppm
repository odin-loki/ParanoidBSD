export module pbsd.port.wave9.hbsd.src.crypto.openssh.ssh_xmss;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/ssh-xmss.c
// void ssh-xmss_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/ssh-xmss.c wave=wave9 loc=389
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_xmss {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_xmss
