export module pbsd.port.wave5.hbsd.src.sys.dev.bhnd.bcma.bcma_erom;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/bhnd/bcma/bcma_erom.c
// void bcma_erom_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/bhnd/bcma/bcma_erom.c wave=wave5 loc=1419
export namespace pbsd::port::wave5::hbsd::src::sys::dev::bhnd::bcma::bcma_erom {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::bhnd::bcma::bcma_erom
