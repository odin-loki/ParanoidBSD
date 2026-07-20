export module pbsd.port.wave2.hbsd.src.lib.libmemstat.memstat_malloc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libmemstat/memstat_malloc.c
// void memstat_malloc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libmemstat/memstat_malloc.c wave=wave2 loc=543
export namespace pbsd::port::wave2::hbsd::src::lib::libmemstat::memstat_malloc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libmemstat::memstat_malloc
