export module pbsd.port.wave9.hbsd.src.contrib.libarchive.libarchive.archive_entry;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libarchive/libarchive/archive_entry.c
// void archive_entry_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libarchive/libarchive/archive_entry.c wave=wave9 loc=2293
export namespace pbsd::port::wave9::hbsd::src::contrib::libarchive::libarchive::archive_entry {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libarchive::libarchive::archive_entry
