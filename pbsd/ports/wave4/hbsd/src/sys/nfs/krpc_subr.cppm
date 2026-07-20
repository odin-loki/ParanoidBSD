export module pbsd.port.wave4.hbsd.src.sys.nfs.krpc_subr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/nfs/krpc_subr.c
// void krpc_subr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/nfs/krpc_subr.c wave=wave4 loc=466
export namespace pbsd::port::wave4::hbsd::src::sys::nfs::krpc_subr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::nfs::krpc_subr
