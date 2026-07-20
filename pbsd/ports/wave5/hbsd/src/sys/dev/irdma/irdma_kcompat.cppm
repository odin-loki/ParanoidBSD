export module pbsd.port.wave5.hbsd.src.sys.dev.irdma.irdma_kcompat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/irdma/irdma_kcompat.c
// void irdma_kcompat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/irdma/irdma_kcompat.c wave=wave5 loc=1944
export namespace pbsd::port::wave5::hbsd::src::sys::dev::irdma::irdma_kcompat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::irdma::irdma_kcompat
