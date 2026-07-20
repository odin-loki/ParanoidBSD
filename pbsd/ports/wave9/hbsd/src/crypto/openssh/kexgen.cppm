export module pbsd.port.wave9.hbsd.src.crypto.openssh.kexgen;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/kexgen.c
// void kexgen_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/kexgen.c wave=wave9 loc=384
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::kexgen {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::kexgen
