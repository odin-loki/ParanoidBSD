export module pbsd.port.wave2.hbsd.src.lib.libc.db.hash.ndbm;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/db/hash/ndbm.c
// void ndbm_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/db/hash/ndbm.c wave=wave2 loc=208
export namespace pbsd::port::wave2::hbsd::src::lib::libc::db::hash::ndbm {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::db::hash::ndbm
