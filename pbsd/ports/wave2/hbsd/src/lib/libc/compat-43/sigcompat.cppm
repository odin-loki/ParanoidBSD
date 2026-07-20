export module pbsd.port.wave2.hbsd.src.lib.libc.compat_43.sigcompat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/compat-43/sigcompat.c
// void sigcompat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/compat-43/sigcompat.c wave=wave2 loc=181
export namespace pbsd::port::wave2::hbsd::src::lib::libc::compat_43::sigcompat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::compat_43::sigcompat
