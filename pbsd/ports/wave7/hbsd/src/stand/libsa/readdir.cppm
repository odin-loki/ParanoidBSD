export module pbsd.port.wave7.hbsd.src.stand.libsa.readdir;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/stand/libsa/readdir.c
// void readdir_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/stand/libsa/readdir.c wave=wave7 loc=49
export namespace pbsd::port::wave7::hbsd::src::stand::libsa::readdir {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::stand::libsa::readdir
