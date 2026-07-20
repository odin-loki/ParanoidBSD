export module pbsd.port.wave5.hbsd.src.sys.dev.psci.smccc_errata;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/psci/smccc_errata.c
// void smccc_errata_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/psci/smccc_errata.c wave=wave5 loc=139
export namespace pbsd::port::wave5::hbsd::src::sys::dev::psci::smccc_errata {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::psci::smccc_errata
