export module pbsd.port.wave2.hbsd.src.libexec.tftpd.tftp_io;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/libexec/tftpd/tftp-io.c
// void tftp-io_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/libexec/tftpd/tftp-io.c wave=wave2 loc=447
export namespace pbsd::port::wave2::hbsd::src::libexec::tftpd::tftp_io {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::libexec::tftpd::tftp_io
