export module pbsd.port.wave2.hbsd.src.lib.libc.stdlib.getopt_long;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdlib/getopt_long.c
// void getopt_long_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdlib/getopt_long.c wave=wave2 loc=606
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::getopt_long {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::getopt_long
