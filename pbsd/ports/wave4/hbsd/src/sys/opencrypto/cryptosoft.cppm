export module pbsd.port.wave4.hbsd.src.sys.opencrypto.cryptosoft;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/opencrypto/cryptosoft.c
// void cryptosoft_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/opencrypto/cryptosoft.c wave=wave4 loc=1754
export namespace pbsd::port::wave4::hbsd::src::sys::opencrypto::cryptosoft {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::opencrypto::cryptosoft
