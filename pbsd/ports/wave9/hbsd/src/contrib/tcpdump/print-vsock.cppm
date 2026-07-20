export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_vsock;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-vsock.c
// void print-vsock_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-vsock.c wave=wave9 loc=260
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_vsock {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_vsock
