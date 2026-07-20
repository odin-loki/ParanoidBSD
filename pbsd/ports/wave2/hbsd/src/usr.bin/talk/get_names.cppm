export module pbsd.port.wave2.hbsd.src.usr_bin.talk.get_names;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/talk/get_names.c
// void get_names_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/talk/get_names.c wave=wave2 loc=111
export namespace pbsd::port::wave2::hbsd::src::usr_bin::talk::get_names {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::talk::get_names
