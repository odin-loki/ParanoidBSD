export module pbsd.port.wave2.hbsd.src.lib.libsysdecode.errno;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libsysdecode/errno.c
// void errno_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libsysdecode/errno.c wave=wave2 loc=90
export namespace pbsd::port::wave2::hbsd::src::lib::libsysdecode::errno {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libsysdecode::errno
