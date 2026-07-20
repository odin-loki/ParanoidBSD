export module pbsd.port.wave5.hbsd.src.sys.dev.mthca.mthca_cq;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mthca/mthca_cq.c
// void mthca_cq_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mthca/mthca_cq.c wave=wave5 loc=981
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mthca::mthca_cq {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mthca::mthca_cq
