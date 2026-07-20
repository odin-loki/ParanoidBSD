export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_unsupported;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-unsupported.c
// void print-unsupported_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-unsupported.c wave=wave9 loc=32
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_unsupported {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_unsupported
