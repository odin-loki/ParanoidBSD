export module pbsd.port.wave4.hbsd.src.sys.contrib.dev.acpica.common.dmtable;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/dev/acpica/common/dmtable.c
// void dmtable_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/dev/acpica/common/dmtable.c wave=wave4 loc=2261
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::dev::acpica::common::dmtable {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::dev::acpica::common::dmtable
