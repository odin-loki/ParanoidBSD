export module pbsd.port.wave2.hbsd.src.lib.libc.string.strcasecmp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/string/strcasecmp.c
// void strcasecmp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: converted
/// @pbsd-migration-meta source=hbsd/src/lib/libc/string/strcasecmp.c wave=wave2 loc=83
export namespace pbsd::port::wave2::hbsd::src::lib::libc::string::strcasecmp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::string::strcasecmp
