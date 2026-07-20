export module pbsd.port.wave2.hbsd.src.lib.liblua.luaconf_local;

module;
// Header bridge — replace #include of hbsd/src/lib/liblua/luaconf.local.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/liblua/luaconf.local.h wave=wave2 loc=25
export namespace pbsd::port::wave2::hbsd::src::lib::liblua::luaconf_local {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::liblua::luaconf_local
