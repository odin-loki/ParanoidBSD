export module pbsd.port.wave7.hbsd.src.stand.efi.loader.arch.amd64.trap;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/stand/efi/loader/arch/amd64/trap.c
// void trap_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/stand/efi/loader/arch/amd64/trap.c wave=wave7 loc=441
export namespace pbsd::port::wave7::hbsd::src::stand::efi::loader::arch::amd64::trap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::stand::efi::loader::arch::amd64::trap
