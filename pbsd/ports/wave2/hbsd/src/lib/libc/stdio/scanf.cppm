export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.scanf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/scanf.c
// void scanf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/scanf.c wave=wave2 loc=74
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::scanf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::scanf
