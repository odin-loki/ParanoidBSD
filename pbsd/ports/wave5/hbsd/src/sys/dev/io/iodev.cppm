export module pbsd.port.wave5.hbsd.src.sys.dev.io.iodev;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/io/iodev.c
// void iodev_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/io/iodev.c wave=wave5 loc=215
export namespace pbsd::port::wave5::hbsd::src::sys::dev::io::iodev {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::io::iodev
