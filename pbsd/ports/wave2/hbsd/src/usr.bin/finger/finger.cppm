export module pbsd.port.wave2.hbsd.src.usr_bin.finger.finger;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/finger/finger.c
// void finger_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/finger/finger.c wave=wave2 loc=394
export namespace pbsd::port::wave2::hbsd::src::usr_bin::finger::finger {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::finger::finger
