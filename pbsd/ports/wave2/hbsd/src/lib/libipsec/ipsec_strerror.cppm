export module pbsd.port.wave2.hbsd.src.lib.libipsec.ipsec_strerror;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libipsec/ipsec_strerror.c
// void ipsec_strerror_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libipsec/ipsec_strerror.c wave=wave2 loc=87
export namespace pbsd::port::wave2::hbsd::src::lib::libipsec::ipsec_strerror {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libipsec::ipsec_strerror
