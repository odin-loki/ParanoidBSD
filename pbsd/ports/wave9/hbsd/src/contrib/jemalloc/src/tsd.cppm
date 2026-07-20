export module pbsd.port.wave9.hbsd.src.contrib.jemalloc.src.tsd;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/jemalloc/src/tsd.c
// void tsd_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/jemalloc/src/tsd.c wave=wave9 loc=549
export namespace pbsd::port::wave9::hbsd::src::contrib::jemalloc::src::tsd {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::jemalloc::src::tsd
