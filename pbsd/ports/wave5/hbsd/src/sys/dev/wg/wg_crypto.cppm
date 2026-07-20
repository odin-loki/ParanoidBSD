export module pbsd.port.wave5.hbsd.src.sys.dev.wg.wg_crypto;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/wg/wg_crypto.c
// void wg_crypto_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/wg/wg_crypto.c wave=wave5 loc=283
export namespace pbsd::port::wave5::hbsd::src::sys::dev::wg::wg_crypto {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::wg::wg_crypto
