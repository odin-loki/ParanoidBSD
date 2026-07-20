export module pbsd.port.wave9.hbsd.src.crypto.openssh.sshsig;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/sshsig.c
// void sshsig_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/sshsig.c wave=wave9 loc=1163
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::sshsig {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::sshsig
