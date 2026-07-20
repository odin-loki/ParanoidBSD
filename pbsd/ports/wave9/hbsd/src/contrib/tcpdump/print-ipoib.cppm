export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_ipoib;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-ipoib.c
// void print-ipoib_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-ipoib.c wave=wave9 loc=123
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_ipoib {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_ipoib
