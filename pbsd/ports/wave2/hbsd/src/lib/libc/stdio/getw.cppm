export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.getw;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/getw.c
// void getw_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/getw.c wave=wave2 loc=43
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::getw {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::getw
