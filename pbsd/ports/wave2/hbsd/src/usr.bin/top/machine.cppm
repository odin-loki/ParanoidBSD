export module pbsd.port.wave2.hbsd.src.usr_bin.top.machine;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/top/machine.c
// void machine_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/top/machine.c wave=wave2 loc=1709
export namespace pbsd::port::wave2::hbsd::src::usr_bin::top::machine {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::top::machine
