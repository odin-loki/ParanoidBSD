export module pbsd.port.wave2.hbsd.src.usr_sbin.ngctl.name;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/ngctl/name.c
// void name_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/ngctl/name.c wave=wave2 loc=77
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::ngctl::name {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::ngctl::name
