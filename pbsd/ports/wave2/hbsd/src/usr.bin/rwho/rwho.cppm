export module pbsd.port.wave2.hbsd.src.usr_bin.rwho.rwho;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/rwho/rwho.c
// void rwho_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/rwho/rwho.c wave=wave2 loc=235
export namespace pbsd::port::wave2::hbsd::src::usr_bin::rwho::rwho {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::rwho::rwho
