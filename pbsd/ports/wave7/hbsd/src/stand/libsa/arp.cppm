export module pbsd.port.wave7.hbsd.src.stand.libsa.arp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/stand/libsa/arp.c
// void arp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/stand/libsa/arp.c wave=wave7 loc=300
export namespace pbsd::port::wave7::hbsd::src::stand::libsa::arp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::stand::libsa::arp
