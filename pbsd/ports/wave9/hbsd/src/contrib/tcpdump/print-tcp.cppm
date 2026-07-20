export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_tcp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-tcp.c
// void print-tcp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-tcp.c wave=wave9 loc=998
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_tcp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_tcp
