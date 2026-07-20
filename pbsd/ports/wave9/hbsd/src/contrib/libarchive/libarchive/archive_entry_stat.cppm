export module pbsd.port.wave9.hbsd.src.contrib.libarchive.libarchive.archive_entry_stat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libarchive/libarchive/archive_entry_stat.c
// void archive_entry_stat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libarchive/libarchive/archive_entry_stat.c wave=wave9 loc=121
export namespace pbsd::port::wave9::hbsd::src::contrib::libarchive::libarchive::archive_entry_stat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libarchive::libarchive::archive_entry_stat
