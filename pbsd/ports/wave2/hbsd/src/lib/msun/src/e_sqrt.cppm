export module pbsd.port.wave2.hbsd.src.lib.msun.src.e_sqrt;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/msun/src/e_sqrt.c
// void e_sqrt_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: converted
/// @pbsd-migration-meta source=hbsd/src/lib/msun/src/e_sqrt.c wave=wave2 loc=455
export namespace pbsd::port::wave2::hbsd::src::lib::msun::src::e_sqrt {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::msun::src::e_sqrt
