export module pbsd.port.wave9.hbsd.src.contrib.ofed.librdmacm.addrinfo;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ofed/librdmacm/addrinfo.c
// void addrinfo_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ofed/librdmacm/addrinfo.c wave=wave9 loc=319
export namespace pbsd::port::wave9::hbsd::src::contrib::ofed::librdmacm::addrinfo {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ofed::librdmacm::addrinfo
