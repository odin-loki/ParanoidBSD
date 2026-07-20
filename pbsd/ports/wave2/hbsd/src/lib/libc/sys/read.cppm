export module pbsd.port.wave2.hbsd.src.lib.libc.sys.read;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/sys/read.c
// void read_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/sys/read.c wave=wave2 loc=42
export namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::read {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::read
