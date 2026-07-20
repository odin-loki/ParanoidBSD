export module pbsd.port.wave9.hbsd.src.crypto.openssh.openbsd_compat.getgrouplist;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/openbsd-compat/getgrouplist.c
// void getgrouplist_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/openbsd-compat/getgrouplist.c wave=wave9 loc=95
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::getgrouplist {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::getgrouplist
