export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_bgp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-bgp.c
// void print-bgp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-bgp.c wave=wave9 loc=3165
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_bgp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_bgp
