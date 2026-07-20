export module pbsd.port.wave2.hbsd.src.lib.libc.stdlib.tsearch;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdlib/tsearch.c
// void tsearch_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdlib/tsearch.c wave=wave2 loc=194
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::tsearch {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::tsearch
