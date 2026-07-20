export module pbsd.port.wave4.hbsd.src.sys.kern.subr_efi_map;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_efi_map.c
// void subr_efi_map_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_efi_map.c wave=wave4 loc=178
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_efi_map {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_efi_map
