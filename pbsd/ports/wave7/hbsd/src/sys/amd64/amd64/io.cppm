export module pbsd.port.wave7.hbsd.src.sys.amd64.amd64.io;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/amd64/amd64/io.c
// void io_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/amd64/amd64/io.c wave=wave7 loc=58
export namespace pbsd::port::wave7::hbsd::src::sys::amd64::amd64::io {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::amd64::amd64::io
