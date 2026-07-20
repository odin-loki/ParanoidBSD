export module pbsd.port.wave2.hbsd.src.lib.libc.sys.sigwaitinfo;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/sys/sigwaitinfo.c
// void sigwaitinfo_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/sys/sigwaitinfo.c wave=wave2 loc=41
export namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::sigwaitinfo {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::sigwaitinfo
