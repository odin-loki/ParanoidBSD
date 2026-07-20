export module pbsd.port.wave4.hbsd.src.sys.kern.subr_sbuf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_sbuf.c
// void subr_sbuf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_sbuf.c wave=wave4 loc=974
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_sbuf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_sbuf
