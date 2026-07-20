export module pbsd.port.wave2.hbsd.src.usr_bin.finger.lprint;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/finger/lprint.c
// void lprint_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/finger/lprint.c wave=wave2 loc=356
export namespace pbsd::port::wave2::hbsd::src::usr_bin::finger::lprint {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::finger::lprint
