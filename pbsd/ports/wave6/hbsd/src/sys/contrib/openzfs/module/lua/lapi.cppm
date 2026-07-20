export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.lua.lapi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/lua/lapi.c
// void lapi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/lua/lapi.c wave=wave6 loc=1323
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::lua::lapi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::lua::lapi
