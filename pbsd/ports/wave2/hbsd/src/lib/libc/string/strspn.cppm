export module pbsd.port.wave2.hbsd.src.lib.libc.string.strspn;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/string/strspn.c
// void strspn_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: converted
/// @pbsd-migration-meta source=hbsd/src/lib/libc/string/strspn.c wave=wave2 loc=70
export namespace pbsd::port::wave2::hbsd::src::lib::libc::string::strspn {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::string::strspn
