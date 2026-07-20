export module pbsd.port.wave2.hbsd.src.usr_bin.top.loadavg;

module;
// Header bridge — replace #include of hbsd/src/usr.bin/top/loadavg.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/top/loadavg.h wave=wave2 loc=25
export namespace pbsd::port::wave2::hbsd::src::usr_bin::top::loadavg {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::top::loadavg
