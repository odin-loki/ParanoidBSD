export module pbsd.port.wave5.hbsd.src.sys.dev.mthca.mthca_reset;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mthca/mthca_reset.c
// void mthca_reset_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mthca/mthca_reset.c wave=wave5 loc=303
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mthca::mthca_reset {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mthca::mthca_reset
