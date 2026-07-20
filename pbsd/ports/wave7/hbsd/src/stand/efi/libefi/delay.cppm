export module pbsd.port.wave7.hbsd.src.stand.efi.libefi.delay;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/stand/efi/libefi/delay.c
// void delay_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/stand/efi/libefi/delay.c wave=wave7 loc=35
export namespace pbsd::port::wave7::hbsd::src::stand::efi::libefi::delay {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::stand::efi::libefi::delay
