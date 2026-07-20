export module pbsd.port.wave2.hbsd.src.lib.libc.string.stpcpy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/string/stpcpy.c
// void stpcpy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: converted
/// @pbsd-migration-meta source=hbsd/src/lib/libc/string/stpcpy.c wave=wave2 loc=44
export namespace pbsd::port::wave2::hbsd::src::lib::libc::string::stpcpy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::string::stpcpy
