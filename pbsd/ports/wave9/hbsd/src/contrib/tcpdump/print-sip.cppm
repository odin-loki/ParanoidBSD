export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_sip;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-sip.c
// void print-sip_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-sip.c wave=wave9 loc=52
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_sip {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_sip
