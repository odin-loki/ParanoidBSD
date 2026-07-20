export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_nflog;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-nflog.c
// void print-nflog_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-nflog.c wave=wave9 loc=239
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_nflog {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_nflog
