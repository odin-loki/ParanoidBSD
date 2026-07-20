export module pbsd.port.wave4.hbsd.src.sys.kern.subr_msgbuf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_msgbuf.c
// void subr_msgbuf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_msgbuf.c wave=wave4 loc=438
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_msgbuf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_msgbuf
