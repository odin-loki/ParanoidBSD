export module pbsd.port.wave6.hbsd.src.sys.netipsec.ipsec_mod;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netipsec/ipsec_mod.c
// void ipsec_mod_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netipsec/ipsec_mod.c wave=wave6 loc=149
export namespace pbsd::port::wave6::hbsd::src::sys::netipsec::ipsec_mod {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netipsec::ipsec_mod
