export module pbsd.port.wave2.hbsd.src.usr_bin.cmp.link;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/cmp/link.c
// void link_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/cmp/link.c wave=wave2 loc=103
export namespace pbsd::port::wave2::hbsd::src::usr_bin::cmp::link {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::cmp::link
