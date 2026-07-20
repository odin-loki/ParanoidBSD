export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_ppp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-ppp.c
// void print-ppp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-ppp.c wave=wave9 loc=1897
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_ppp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_ppp
