export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.gdb;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/gdb.c
// void gdb_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/gdb.c wave=wave2 loc=2272
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::gdb {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::gdb
