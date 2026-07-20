export module pbsd.port.wave4.hbsd.src.sys.contrib.dev.acpica.common.dmtables;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/dev/acpica/common/dmtables.c
// void dmtables_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/dev/acpica/common/dmtables.c wave=wave4 loc=619
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::dev::acpica::common::dmtables {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::dev::acpica::common::dmtables
