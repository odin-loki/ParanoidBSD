export module pbsd.port.wave2.hbsd.src.usr_bin.patch.inp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/patch/inp.c
// void inp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/patch/inp.c wave=wave2 loc=432
export namespace pbsd::port::wave2::hbsd::src::usr_bin::patch::inp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::patch::inp
