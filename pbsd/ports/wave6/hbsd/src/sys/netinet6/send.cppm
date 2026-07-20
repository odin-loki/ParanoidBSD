export module pbsd.port.wave6.hbsd.src.sys.netinet6.send;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet6/send.c
// void send_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet6/send.c wave=wave6 loc=387
export namespace pbsd::port::wave6::hbsd::src::sys::netinet6::send {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet6::send
