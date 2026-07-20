export module pbsd.port.wave9.hbsd.src.crypto.openssh.sftp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/sftp.c
// void sftp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/sftp.c wave=wave9 loc=2701
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::sftp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::sftp
