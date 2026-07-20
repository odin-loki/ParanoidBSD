export module pbsd.port.wave2.hbsd.src.libexec.tftpd.tftpd;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/libexec/tftpd/tftpd.c
// void tftpd_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/libexec/tftpd/tftpd.c wave=wave2 loc=849
export namespace pbsd::port::wave2::hbsd::src::libexec::tftpd::tftpd {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::libexec::tftpd::tftpd
