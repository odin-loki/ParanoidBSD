export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.lib.libspl.backtrace;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/lib/libspl/backtrace.c
// void backtrace_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/lib/libspl/backtrace.c wave=wave6 loc=303
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libspl::backtrace {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libspl::backtrace
