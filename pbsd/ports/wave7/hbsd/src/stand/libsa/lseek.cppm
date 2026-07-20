export module pbsd.port.wave7.hbsd.src.stand.libsa.lseek;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/stand/libsa/lseek.c
// void lseek_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/stand/libsa/lseek.c wave=wave7 loc=138
export namespace pbsd::port::wave7::hbsd::src::stand::libsa::lseek {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::stand::libsa::lseek
