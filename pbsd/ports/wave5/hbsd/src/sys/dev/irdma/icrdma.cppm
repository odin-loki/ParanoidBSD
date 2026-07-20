export module pbsd.port.wave5.hbsd.src.sys.dev.irdma.icrdma;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/irdma/icrdma.c
// void icrdma_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/irdma/icrdma.c wave=wave5 loc=838
export namespace pbsd::port::wave5::hbsd::src::sys::dev::irdma::icrdma {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::irdma::icrdma
