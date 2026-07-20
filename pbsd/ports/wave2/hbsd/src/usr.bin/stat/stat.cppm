export module pbsd.port.wave2.hbsd.src.usr_bin.stat.stat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/stat/stat.c
// void stat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/stat/stat.c wave=wave2 loc=1193
export namespace pbsd::port::wave2::hbsd::src::usr_bin::stat::stat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::stat::stat
