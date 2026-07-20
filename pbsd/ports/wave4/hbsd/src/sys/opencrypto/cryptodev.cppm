export module pbsd.port.wave4.hbsd.src.sys.opencrypto.cryptodev;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/opencrypto/cryptodev.c
// void cryptodev_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/opencrypto/cryptodev.c wave=wave4 loc=1301
export namespace pbsd::port::wave4::hbsd::src::sys::opencrypto::cryptodev {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::opencrypto::cryptodev
