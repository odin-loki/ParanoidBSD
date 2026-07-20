export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.lib.libspl.mkdirp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/lib/libspl/mkdirp.c
// void mkdirp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/lib/libspl/mkdirp.c wave=wave6 loc=213
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libspl::mkdirp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libspl::mkdirp
