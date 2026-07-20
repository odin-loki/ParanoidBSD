export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_nfs;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-nfs.c
// void print-nfs_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-nfs.c wave=wave9 loc=1859
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_nfs {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_nfs
