export module pbsd.port.wave5.hbsd.src.sys.dev.intel.spi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/intel/spi.c
// void spi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/intel/spi.c wave=wave5 loc=623
export namespace pbsd::port::wave5::hbsd::src::sys::dev::intel::spi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::intel::spi
