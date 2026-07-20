export module pbsd.port.wave2.hbsd.src.usr_bin.top.top;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/top/top.c
// void top_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/top/top.c wave=wave2 loc=1229
export namespace pbsd::port::wave2::hbsd::src::usr_bin::top::top {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::top::top
