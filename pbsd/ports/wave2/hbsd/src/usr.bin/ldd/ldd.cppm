export module pbsd.port.wave2.hbsd.src.usr_bin.ldd.ldd;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/ldd/ldd.c
// void ldd_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/ldd/ldd.c wave=wave2 loc=471
export namespace pbsd::port::wave2::hbsd::src::usr_bin::ldd::ldd {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::ldd::ldd
