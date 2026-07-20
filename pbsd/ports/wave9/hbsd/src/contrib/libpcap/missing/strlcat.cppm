export module pbsd.port.wave9.hbsd.src.contrib.libpcap.missing.strlcat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/missing/strlcat.c
// void strlcat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/missing/strlcat.c wave=wave9 loc=59
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::missing::strlcat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::missing::strlcat
