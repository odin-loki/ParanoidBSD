export module pbsd.port.wave4.hbsd.src.sys.kern.subr_sfbuf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_sfbuf.c
// void subr_sfbuf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_sfbuf.c wave=wave4 loc=230
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_sfbuf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_sfbuf
