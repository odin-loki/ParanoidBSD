export module pbsd.port.wave9.hbsd.src.crypto.libecc.src.sig.ecrdsa;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/libecc/src/sig/ecrdsa.c
// void ecrdsa_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/libecc/src/sig/ecrdsa.c wave=wave9 loc=623
export namespace pbsd::port::wave9::hbsd::src::crypto::libecc::src::sig::ecrdsa {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::libecc::src::sig::ecrdsa
