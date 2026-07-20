export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_arp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-arp.c
// void print-arp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-arp.c wave=wave9 loc=471
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_arp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_arp
