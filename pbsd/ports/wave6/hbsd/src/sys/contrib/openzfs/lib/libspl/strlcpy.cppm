export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.lib.libspl.strlcpy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/lib/libspl/strlcpy.c
// void strlcpy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/lib/libspl/strlcpy.c wave=wave6 loc=57
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libspl::strlcpy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libspl::strlcpy
