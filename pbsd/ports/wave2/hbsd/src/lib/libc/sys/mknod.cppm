export module pbsd.port.wave2.hbsd.src.lib.libc.sys.mknod;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/sys/mknod.c
// void mknod_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/sys/mknod.c wave=wave2 loc=42
export namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::mknod {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::mknod
