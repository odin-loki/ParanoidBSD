export module pbsd.port.wave2.hbsd.src.lib.libc.stdlib.qsort_s;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdlib/qsort_s.c
// void qsort_s_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdlib/qsort_s.c wave=wave2 loc=6
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::qsort_s {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::qsort_s
