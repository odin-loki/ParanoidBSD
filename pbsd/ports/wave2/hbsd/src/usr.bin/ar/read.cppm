export module pbsd.port.wave2.hbsd.src.usr_bin.ar.read;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/ar/read.c
// void read_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/ar/read.c wave=wave2 loc=204
export namespace pbsd::port::wave2::hbsd::src::usr_bin::ar::read {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::ar::read
