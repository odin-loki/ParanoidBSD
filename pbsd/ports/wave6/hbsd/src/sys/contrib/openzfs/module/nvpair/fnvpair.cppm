export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.nvpair.fnvpair;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/nvpair/fnvpair.c
// void fnvpair_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/nvpair/fnvpair.c wave=wave6 loc=667
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::nvpair::fnvpair {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::nvpair::fnvpair
