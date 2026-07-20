export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.nvpair.nvpair;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/nvpair/nvpair.c
// void nvpair_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/nvpair/nvpair.c wave=wave6 loc=4059
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::nvpair::nvpair {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::nvpair::nvpair
