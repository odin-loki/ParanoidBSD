export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_someip;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-someip.c
// void print-someip_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-someip.c wave=wave9 loc=139
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_someip {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_someip
