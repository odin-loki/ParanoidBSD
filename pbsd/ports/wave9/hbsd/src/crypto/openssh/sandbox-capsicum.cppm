export module pbsd.port.wave9.hbsd.src.crypto.openssh.sandbox_capsicum;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/sandbox-capsicum.c
// void sandbox-capsicum_init(void);
}

export import pbsd.userland.capsicum;
export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/sandbox-capsicum.c wave=wave9 loc=110
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::sandbox_capsicum {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::sandbox_capsicum
