export module pbsd.port.wave2.hbsd.src.lib.libproc.proc_bkpt;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libproc/proc_bkpt.c
// void proc_bkpt_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libproc/proc_bkpt.c wave=wave2 loc=262
export namespace pbsd::port::wave2::hbsd::src::lib::libproc::proc_bkpt {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libproc::proc_bkpt
