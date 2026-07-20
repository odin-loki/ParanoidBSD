export module pbsd.port.wave2.hbsd.src.lib.libc.string.strcspn;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/string/strcspn.c
// void strcspn_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: converted
/// @pbsd-migration-meta source=hbsd/src/lib/libc/string/strcspn.c wave=wave2 loc=71
export namespace pbsd::port::wave2::hbsd::src::lib::libc::string::strcspn {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::string::strcspn
