export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.icp.illumos_crypto;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/icp/illumos-crypto.c
// void illumos-crypto_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/icp/illumos-crypto.c wave=wave6 loc=139
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::icp::illumos_crypto {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::icp::illumos_crypto
