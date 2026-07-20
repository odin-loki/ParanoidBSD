export module pbsd.port.wave6.hbsd.src.sys.fs.nullfs.null_subr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/fs/nullfs/null_subr.c
// void null_subr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/fs/nullfs/null_subr.c wave=wave6 loc=320
export namespace pbsd::port::wave6::hbsd::src::sys::fs::nullfs::null_subr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::fs::nullfs::null_subr
