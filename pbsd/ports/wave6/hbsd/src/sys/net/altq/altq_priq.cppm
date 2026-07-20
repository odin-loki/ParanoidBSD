export module pbsd.port.wave6.hbsd.src.sys.net.altq.altq_priq;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/altq/altq_priq.c
// void altq_priq_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/altq/altq_priq.c wave=wave6 loc=642
export namespace pbsd::port::wave6::hbsd::src::sys::net::altq::altq_priq {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::altq::altq_priq
