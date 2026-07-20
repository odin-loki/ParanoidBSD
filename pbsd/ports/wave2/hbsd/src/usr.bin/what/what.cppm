export module pbsd.port.wave2.hbsd.src.usr_bin.what.what;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/what/what.c
// void what_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/what/what.c wave=wave2 loc=125
export namespace pbsd::port::wave2::hbsd::src::usr_bin::what::what {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::what::what
