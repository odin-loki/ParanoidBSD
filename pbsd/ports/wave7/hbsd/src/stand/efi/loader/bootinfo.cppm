export module pbsd.port.wave7.hbsd.src.stand.efi.loader.bootinfo;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/stand/efi/loader/bootinfo.c
// void bootinfo_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/stand/efi/loader/bootinfo.c wave=wave7 loc=512
export namespace pbsd::port::wave7::hbsd::src::stand::efi::loader::bootinfo {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::stand::efi::loader::bootinfo
