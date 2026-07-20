export module pbsd.port.wave2.hbsd.src.usr_bin.tftp.tftp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/tftp/tftp.c
// void tftp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/tftp/tftp.c wave=wave2 loc=272
export namespace pbsd::port::wave2::hbsd::src::usr_bin::tftp::tftp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::tftp::tftp
