export module pbsd.port.wave2.hbsd.src.lib.msun.src.s_scalbn;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/msun/src/s_scalbn.c
// void s_scalbn_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/msun/src/s_scalbn.c wave=wave2 loc=47
export namespace pbsd::port::wave2::hbsd::src::lib::msun::src::s_scalbn {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::msun::src::s_scalbn
