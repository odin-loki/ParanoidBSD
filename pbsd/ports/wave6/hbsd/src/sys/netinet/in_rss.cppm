export module pbsd.port.wave6.hbsd.src.sys.netinet.in_rss;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/in_rss.c
// void in_rss_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/in_rss.c wave=wave6 loc=389
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::in_rss {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::in_rss
