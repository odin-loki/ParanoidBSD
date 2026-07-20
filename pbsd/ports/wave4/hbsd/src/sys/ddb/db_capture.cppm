export module pbsd.port.wave4.hbsd.src.sys.ddb.db_capture;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/ddb/db_capture.c
// void db_capture_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/ddb/db_capture.c wave=wave4 loc=364
export namespace pbsd::port::wave4::hbsd::src::sys::ddb::db_capture {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::ddb::db_capture
