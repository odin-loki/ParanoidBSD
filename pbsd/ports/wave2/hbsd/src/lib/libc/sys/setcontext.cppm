export module pbsd.port.wave2.hbsd.src.lib.libc.sys.setcontext;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/sys/setcontext.c
// void setcontext_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/sys/setcontext.c wave=wave2 loc=45
export namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::setcontext {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::setcontext
