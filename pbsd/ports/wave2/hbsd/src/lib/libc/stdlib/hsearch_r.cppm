export module pbsd.port.wave2.hbsd.src.lib.libc.stdlib.hsearch_r;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdlib/hsearch_r.c
// void hsearch_r_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdlib/hsearch_r.c wave=wave2 loc=147
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::hsearch_r {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::hsearch_r
