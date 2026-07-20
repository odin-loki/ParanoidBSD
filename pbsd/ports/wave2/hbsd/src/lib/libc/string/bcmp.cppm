export module pbsd.port.wave2.hbsd.src.lib.libc.string.bcmp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/string/bcmp.c
// void bcmp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: converted
/// @pbsd-migration-meta source=hbsd/src/lib/libc/string/bcmp.c wave=wave2 loc=51
export namespace pbsd::port::wave2::hbsd::src::lib::libc::string::bcmp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::string::bcmp
