export module pbsd.port.wave5.hbsd.src.sys.dev.irdma.irdma_verbs;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/irdma/irdma_verbs.c
// void irdma_verbs_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/irdma/irdma_verbs.c wave=wave5 loc=3499
export namespace pbsd::port::wave5::hbsd::src::sys::dev::irdma::irdma_verbs {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::irdma::irdma_verbs
