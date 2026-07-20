export module pbsd.port.wave4.hbsd.src.sys.contrib.libfdt.fdt;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/libfdt/fdt.c
// void fdt_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/libfdt/fdt.c wave=wave4 loc=254
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::libfdt::fdt {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::libfdt::fdt
