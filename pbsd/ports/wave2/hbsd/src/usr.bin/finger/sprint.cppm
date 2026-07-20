export module pbsd.port.wave2.hbsd.src.usr_bin.finger.sprint;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/finger/sprint.c
// void sprint_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/finger/sprint.c wave=wave2 loc=177
export namespace pbsd::port::wave2::hbsd::src::usr_bin::finger::sprint {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::finger::sprint
