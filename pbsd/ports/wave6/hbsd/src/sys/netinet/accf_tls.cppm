export module pbsd.port.wave6.hbsd.src.sys.netinet.accf_tls;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/accf_tls.c
// void accf_tls_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/accf_tls.c wave=wave6 loc=106
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::accf_tls {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::accf_tls
