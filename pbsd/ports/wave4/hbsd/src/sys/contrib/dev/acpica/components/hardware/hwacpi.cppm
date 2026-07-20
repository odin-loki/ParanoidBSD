export module pbsd.port.wave4.hbsd.src.sys.contrib.dev.acpica.components.hardware.hwacpi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/dev/acpica/components/hardware/hwacpi.c
// void hwacpi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/dev/acpica/components/hardware/hwacpi.c wave=wave4 loc=329
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::dev::acpica::components::hardware::hwacpi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::dev::acpica::components::hardware::hwacpi
