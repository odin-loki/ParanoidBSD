export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_pppoe;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-pppoe.c
// void print-pppoe_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-pppoe.c wave=wave9 loc=204
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_pppoe {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_pppoe
