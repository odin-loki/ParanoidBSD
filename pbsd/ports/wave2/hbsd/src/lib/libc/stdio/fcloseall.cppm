export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.fcloseall;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/fcloseall.c
// void fcloseall_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/fcloseall.c wave=wave2 loc=38
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::fcloseall {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::fcloseall
