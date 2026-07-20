export module pbsd.port.wave4.hbsd.src.sys.gdb.netgdb;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/gdb/netgdb.c
// void netgdb_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/gdb/netgdb.c wave=wave4 loc=390
export namespace pbsd::port::wave4::hbsd::src::sys::gdb::netgdb {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::gdb::netgdb
