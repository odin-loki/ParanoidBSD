export module pbsd.port.wave6.hbsd.src.sys.net.rss_config;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/rss_config.c
// void rss_config_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/rss_config.c wave=wave6 loc=546
export namespace pbsd::port::wave6::hbsd::src::sys::net::rss_config {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::rss_config
