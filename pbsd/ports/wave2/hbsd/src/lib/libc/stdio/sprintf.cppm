export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.sprintf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/sprintf.c
// void sprintf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/sprintf.c wave=wave2 loc=71
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::sprintf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::sprintf
