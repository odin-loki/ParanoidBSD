export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_sll;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-sll.c
// void print-sll_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-sll.c wave=wave9 loc=533
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_sll {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_sll
