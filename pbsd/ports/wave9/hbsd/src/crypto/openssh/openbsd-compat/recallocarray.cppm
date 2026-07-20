export module pbsd.port.wave9.hbsd.src.crypto.openssh.openbsd_compat.recallocarray;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/openbsd-compat/recallocarray.c
// void recallocarray_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/openbsd-compat/recallocarray.c wave=wave9 loc=90
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::recallocarray {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::recallocarray
