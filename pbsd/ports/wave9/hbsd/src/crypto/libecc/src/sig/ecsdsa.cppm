export module pbsd.port.wave9.hbsd.src.crypto.libecc.src.sig.ecsdsa;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/libecc/src/sig/ecsdsa.c
// void ecsdsa_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/libecc/src/sig/ecsdsa.c wave=wave9 loc=97
export namespace pbsd::port::wave9::hbsd::src::crypto::libecc::src::sig::ecsdsa {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::libecc::src::sig::ecsdsa
