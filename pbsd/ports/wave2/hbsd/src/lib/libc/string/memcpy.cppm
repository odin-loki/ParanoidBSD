export module pbsd.port.wave2.hbsd.src.lib.libc.string.memcpy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/string/memcpy.c
// void memcpy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/string/memcpy.c wave=wave2 loc=2
export namespace pbsd::port::wave2::hbsd::src::lib::libc::string::memcpy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::string::memcpy
