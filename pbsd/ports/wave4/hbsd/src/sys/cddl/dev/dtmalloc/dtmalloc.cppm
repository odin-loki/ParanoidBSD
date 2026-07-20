export module pbsd.port.wave4.hbsd.src.sys.cddl.dev.dtmalloc.dtmalloc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/cddl/dev/dtmalloc/dtmalloc.c
// void dtmalloc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/cddl/dev/dtmalloc/dtmalloc.c wave=wave4 loc=216
export namespace pbsd::port::wave4::hbsd::src::sys::cddl::dev::dtmalloc::dtmalloc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::cddl::dev::dtmalloc::dtmalloc
