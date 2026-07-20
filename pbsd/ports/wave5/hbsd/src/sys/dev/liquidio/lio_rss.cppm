export module pbsd.port.wave5.hbsd.src.sys.dev.liquidio.lio_rss;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/liquidio/lio_rss.c
// void lio_rss_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/liquidio/lio_rss.c wave=wave5 loc=172
export namespace pbsd::port::wave5::hbsd::src::sys::dev::liquidio::lio_rss {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::liquidio::lio_rss
