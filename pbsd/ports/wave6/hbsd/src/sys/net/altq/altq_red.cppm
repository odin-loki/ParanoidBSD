export module pbsd.port.wave6.hbsd.src.sys.net.altq.altq_red;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/altq/altq_red.c
// void altq_red_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/altq/altq_red.c wave=wave6 loc=627
export namespace pbsd::port::wave6::hbsd::src::sys::net::altq::altq_red {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::altq::altq_red
