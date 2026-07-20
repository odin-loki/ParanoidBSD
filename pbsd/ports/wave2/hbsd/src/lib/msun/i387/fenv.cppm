export module pbsd.port.wave2.hbsd.src.lib.msun.i387.fenv;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/msun/i387/fenv.c
// void fenv_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/msun/i387/fenv.c wave=wave2 loc=249
export namespace pbsd::port::wave2::hbsd::src::lib::msun::i387::fenv {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::msun::i387::fenv
