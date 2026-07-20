export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.module.lua.lcompat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/module/lua/lcompat.c
// void lcompat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/module/lua/lcompat.c wave=wave6 loc=102
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::lua::lcompat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::module::lua::lcompat
