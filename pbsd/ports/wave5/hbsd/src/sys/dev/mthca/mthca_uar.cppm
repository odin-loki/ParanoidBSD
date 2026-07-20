export module pbsd.port.wave5.hbsd.src.sys.dev.mthca.mthca_uar;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mthca/mthca_uar.c
// void mthca_uar_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mthca/mthca_uar.c wave=wave5 loc=76
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mthca::mthca_uar {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mthca::mthca_uar
