export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_loopback;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-loopback.c
// void print-loopback_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-loopback.c wave=wave9 loc=134
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_loopback {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_loopback
