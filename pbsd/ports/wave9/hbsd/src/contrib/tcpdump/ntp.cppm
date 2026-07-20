export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.ntp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/ntp.c
// void ntp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/ntp.c wave=wave9 loc=86
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::ntp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::ntp
