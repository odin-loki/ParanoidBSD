export module pbsd.port.wave5.hbsd.src.sys.dev.mthca.mthca_eq;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mthca/mthca_eq.c
// void mthca_eq_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mthca/mthca_eq.c wave=wave5 loc=905
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mthca::mthca_eq {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mthca::mthca_eq
