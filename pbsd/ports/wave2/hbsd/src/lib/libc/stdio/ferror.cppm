export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.ferror;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/ferror.c
// void ferror_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/ferror.c wave=wave2 loc=59
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::ferror {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::ferror
