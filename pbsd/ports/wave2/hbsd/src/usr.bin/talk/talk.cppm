export module pbsd.port.wave2.hbsd.src.usr_bin.talk.talk;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/talk/talk.c
// void talk_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/talk/talk.c wave=wave2 loc=70
export namespace pbsd::port::wave2::hbsd::src::usr_bin::talk::talk {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::talk::talk
