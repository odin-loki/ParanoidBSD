export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.lua.lmem;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/lua/lmem.c
// void lmem_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/lua/lmem.c wave=wave6 loc=97
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::lua::lmem {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::lua::lmem
