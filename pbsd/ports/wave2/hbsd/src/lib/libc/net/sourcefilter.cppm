export module pbsd.port.wave2.hbsd.src.lib.libc.net.sourcefilter;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/net/sourcefilter.c
// void sourcefilter_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/net/sourcefilter.c wave=wave2 loc=401
export namespace pbsd::port::wave2::hbsd::src::lib::libc::net::sourcefilter {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::net::sourcefilter
