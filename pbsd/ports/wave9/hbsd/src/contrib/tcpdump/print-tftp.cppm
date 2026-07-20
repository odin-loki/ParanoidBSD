export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_tftp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-tftp.c
// void print-tftp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-tftp.c wave=wave9 loc=196
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_tftp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_tftp
