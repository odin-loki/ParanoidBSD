export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.lua.lzio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/lua/lzio.c
// void lzio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/lua/lzio.c wave=wave6 loc=73
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::lua::lzio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::lua::lzio
