export module pbsd.port.wave9.hbsd.src.crypto.openssh.xmalloc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/xmalloc.c
// void xmalloc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/xmalloc.c wave=wave9 loc=118
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::xmalloc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::xmalloc
