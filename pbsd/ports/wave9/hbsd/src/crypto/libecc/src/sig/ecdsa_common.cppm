export module pbsd.port.wave9.hbsd.src.crypto.libecc.src.sig.ecdsa_common;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/libecc/src/sig/ecdsa_common.c
// void ecdsa_common_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/libecc/src/sig/ecdsa_common.c wave=wave9 loc=1039
export namespace pbsd::port::wave9::hbsd::src::crypto::libecc::src::sig::ecdsa_common {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::libecc::src::sig::ecdsa_common
