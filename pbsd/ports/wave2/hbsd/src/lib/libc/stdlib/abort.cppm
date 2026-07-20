export module pbsd.port.wave2.hbsd.src.lib.libc.stdlib.abort;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdlib/abort.c
// void abort_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdlib/abort.c wave=wave2 loc=75
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::abort {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::abort
