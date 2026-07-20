export module pbsd.port.wave7.hbsd.src.stand.efi.loader.autoload;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/stand/efi/loader/autoload.c
// void autoload_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/stand/efi/loader/autoload.c wave=wave7 loc=54
export namespace pbsd::port::wave7::hbsd::src::stand::efi::loader::autoload {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::stand::efi::loader::autoload
