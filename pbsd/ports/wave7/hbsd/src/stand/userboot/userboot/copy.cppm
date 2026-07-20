export module pbsd.port.wave7.hbsd.src.stand.userboot.userboot.copy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/stand/userboot/userboot/copy.c
// void copy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/stand/userboot/userboot/copy.c wave=wave7 loc=70
export namespace pbsd::port::wave7::hbsd::src::stand::userboot::userboot::copy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::stand::userboot::userboot::copy
