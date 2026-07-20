export module pbsd.port.wave2.hbsd.src.lib.libc.string.strsep;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/string/strsep.c
// void strsep_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: converted
/// @pbsd-migration-meta source=hbsd/src/lib/libc/string/strsep.c wave=wave2 loc=71
export namespace pbsd::port::wave2::hbsd::src::lib::libc::string::strsep {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::string::strsep
