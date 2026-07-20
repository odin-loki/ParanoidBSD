export module pbsd.port.wave5.hbsd.src.sys.dev.ioat.ioat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ioat/ioat.c
// void ioat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ioat/ioat.c wave=wave5 loc=2163
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ioat::ioat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ioat::ioat
