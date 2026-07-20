export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.fread;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/fread.c
// void fread_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/fread.c wave=wave2 loc=146
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::fread {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::fread
