export module pbsd.port.wave2.hbsd.src.lib.libc.string.strcpy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/string/strcpy.c
// void strcpy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/string/strcpy.c wave=wave2 loc=50
export namespace pbsd::port::wave2::hbsd::src::lib::libc::string::strcpy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::string::strcpy
