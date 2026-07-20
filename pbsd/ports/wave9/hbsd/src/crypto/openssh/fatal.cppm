export module pbsd.port.wave9.hbsd.src.crypto.openssh.fatal;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/fatal.c
// void fatal_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/fatal.c wave=wave9 loc=46
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::fatal {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::fatal
