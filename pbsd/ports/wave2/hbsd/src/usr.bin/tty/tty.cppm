export module pbsd.port.wave2.hbsd.src.usr_bin.tty.tty;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/tty/tty.c
// void tty_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/tty/tty.c wave=wave2 loc=67
export namespace pbsd::port::wave2::hbsd::src::usr_bin::tty::tty {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::tty::tty
