export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.lib.libspl.strlcat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/lib/libspl/strlcat.c
// void strlcat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/lib/libspl/strlcat.c wave=wave6 loc=61
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libspl::strlcat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libspl::strlcat
