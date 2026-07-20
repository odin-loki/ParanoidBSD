export module pbsd.port.wave2.hbsd.src.libexec.tftpd.tftp_file;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/libexec/tftpd/tftp-file.c
// void tftp-file_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/libexec/tftpd/tftp-file.c wave=wave2 loc=299
export namespace pbsd::port::wave2::hbsd::src::libexec::tftpd::tftp_file {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::libexec::tftpd::tftp_file
