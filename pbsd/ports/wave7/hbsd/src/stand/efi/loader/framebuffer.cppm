export module pbsd.port.wave7.hbsd.src.stand.efi.loader.framebuffer;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/stand/efi/loader/framebuffer.c
// void framebuffer_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/stand/efi/loader/framebuffer.c wave=wave7 loc=1006
export namespace pbsd::port::wave7::hbsd::src::stand::efi::loader::framebuffer {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::stand::efi::loader::framebuffer
