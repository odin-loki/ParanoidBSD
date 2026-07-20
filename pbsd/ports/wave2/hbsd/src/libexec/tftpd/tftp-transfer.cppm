export module pbsd.port.wave2.hbsd.src.libexec.tftpd.tftp_transfer;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/libexec/tftpd/tftp-transfer.c
// void tftp-transfer_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/libexec/tftpd/tftp-transfer.c wave=wave2 loc=446
export namespace pbsd::port::wave2::hbsd::src::libexec::tftpd::tftp_transfer {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::libexec::tftpd::tftp_transfer
