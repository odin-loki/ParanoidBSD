export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.missing.getopt_long;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/missing/getopt_long.c
// void getopt_long_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/missing/getopt_long.c wave=wave9 loc=635
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::missing::getopt_long {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::missing::getopt_long
