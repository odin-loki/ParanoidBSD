export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.tpm_emul_passthru;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/tpm_emul_passthru.c
// void tpm_emul_passthru_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/tpm_emul_passthru.c wave=wave2 loc=107
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::tpm_emul_passthru {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::tpm_emul_passthru
