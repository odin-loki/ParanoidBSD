export module pbsd.port.wave2.hbsd.src.usr_bin.who.who;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/who/who.c
// void who_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/who/who.c wave=wave2 loc=315
export namespace pbsd::port::wave2::hbsd::src::usr_bin::who::who {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::who::who
