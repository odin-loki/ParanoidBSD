export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_ntp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-ntp.c
// void print-ntp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-ntp.c wave=wave9 loc=532
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_ntp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_ntp
