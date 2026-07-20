export module pbsd.port.wave2.hbsd.src.usr_bin.rusers.rusers;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/rusers/rusers.c
// void rusers_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/rusers/rusers.c wave=wave2 loc=250
export namespace pbsd::port::wave2::hbsd::src::usr_bin::rusers::rusers {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::rusers::rusers
