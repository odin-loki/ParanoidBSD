export module pbsd.port.wave5.hbsd.src.sys.dev.superio.superio_io;

module;
// Header bridge — replace #include of hbsd/src/sys/dev/superio/superio_io.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/superio/superio_io.h wave=wave5 loc=42
export namespace pbsd::port::wave5::hbsd::src::sys::dev::superio::superio_io {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::superio::superio_io
