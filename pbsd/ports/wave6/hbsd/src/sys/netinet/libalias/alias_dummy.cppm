export module pbsd.port.wave6.hbsd.src.sys.netinet.libalias.alias_dummy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet/libalias/alias_dummy.c
// void alias_dummy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet/libalias/alias_dummy.c wave=wave6 loc=149
export namespace pbsd::port::wave6::hbsd::src::sys::netinet::libalias::alias_dummy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet::libalias::alias_dummy
