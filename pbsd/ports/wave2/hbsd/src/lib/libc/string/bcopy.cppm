export module pbsd.port.wave2.hbsd.src.lib.libc.string.bcopy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/string/bcopy.c
// void bcopy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/string/bcopy.c wave=wave2 loc=136
export namespace pbsd::port::wave2::hbsd::src::lib::libc::string::bcopy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::string::bcopy
