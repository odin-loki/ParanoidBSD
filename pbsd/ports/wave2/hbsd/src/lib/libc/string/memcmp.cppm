export module pbsd.port.wave2.hbsd.src.lib.libc.string.memcmp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/string/memcmp.c
// void memcmp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/string/memcmp.c wave=wave2 loc=52
export namespace pbsd::port::wave2::hbsd::src::lib::libc::string::memcmp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::string::memcmp
