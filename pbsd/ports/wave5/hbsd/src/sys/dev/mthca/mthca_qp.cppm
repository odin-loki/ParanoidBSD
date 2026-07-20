export module pbsd.port.wave5.hbsd.src.sys.dev.mthca.mthca_qp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mthca/mthca_qp.c
// void mthca_qp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mthca/mthca_qp.c wave=wave5 loc=2317
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mthca::mthca_qp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mthca::mthca_qp
