export module pbsd.port.wave4.hbsd.src.sys.opencrypto.rmd160;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/opencrypto/rmd160.c
// void rmd160_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/opencrypto/rmd160.c wave=wave4 loc=364
export namespace pbsd::port::wave4::hbsd::src::sys::opencrypto::rmd160 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::opencrypto::rmd160
