export module pbsd.port.wave2.hbsd.src.lib.libfetch.ftp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libfetch/ftp.c
// void ftp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libfetch/ftp.c wave=wave2 loc=1211
export namespace pbsd::port::wave2::hbsd::src::lib::libfetch::ftp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libfetch::ftp
