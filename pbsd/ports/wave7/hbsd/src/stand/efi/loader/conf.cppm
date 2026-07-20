export module pbsd.port.wave7.hbsd.src.stand.efi.loader.conf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/stand/efi/loader/conf.c
// void conf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/stand/efi/loader/conf.c wave=wave7 loc=103
export namespace pbsd::port::wave7::hbsd::src::stand::efi::loader::conf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::stand::efi::loader::conf
