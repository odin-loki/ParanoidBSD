export module pbsd.port.wave5.hbsd.src.sys.dev.tpm.tpm_crb;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/tpm/tpm_crb.c
// void tpm_crb_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/tpm/tpm_crb.c wave=wave5 loc=662
export namespace pbsd::port::wave5::hbsd::src::sys::dev::tpm::tpm_crb {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::tpm::tpm_crb
