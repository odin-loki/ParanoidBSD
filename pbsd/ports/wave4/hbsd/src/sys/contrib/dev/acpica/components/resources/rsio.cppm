export module pbsd.port.wave4.hbsd.src.sys.contrib.dev.acpica.components.resources.rsio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/dev/acpica/components/resources/rsio.c
// void rsio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/dev/acpica/components/resources/rsio.c wave=wave4 loc=408
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::dev::acpica::components::resources::rsio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::dev::acpica::components::resources::rsio
