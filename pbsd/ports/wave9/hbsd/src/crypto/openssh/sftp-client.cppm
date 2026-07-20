export module pbsd.port.wave9.hbsd.src.crypto.openssh.sftp_client;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/sftp-client.c
// void sftp-client_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/sftp-client.c wave=wave9 loc=3010
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::sftp_client {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::sftp_client
