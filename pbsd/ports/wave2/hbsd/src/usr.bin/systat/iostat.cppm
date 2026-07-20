export module pbsd.port.wave2.hbsd.src.usr_bin.systat.iostat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/systat/iostat.c
// void iostat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/systat/iostat.c wave=wave2 loc=384
export namespace pbsd::port::wave2::hbsd::src::usr_bin::systat::iostat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::systat::iostat
