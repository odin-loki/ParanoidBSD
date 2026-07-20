export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.icp.algs.skein.skein;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/icp/algs/skein/skein.c
// void skein_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/icp/algs/skein/skein.c wave=wave6 loc=906
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::icp::algs::skein::skein {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::icp::algs::skein::skein
