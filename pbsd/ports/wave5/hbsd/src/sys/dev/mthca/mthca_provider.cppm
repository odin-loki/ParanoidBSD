export module pbsd.port.wave5.hbsd.src.sys.dev.mthca.mthca_provider;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mthca/mthca_provider.c
// void mthca_provider_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mthca/mthca_provider.c wave=wave5 loc=1272
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mthca::mthca_provider {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mthca::mthca_provider
