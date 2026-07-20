export module pbsd.port.wave4.hbsd.src.sys.kern.uipc_accf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/uipc_accf.c
// void uipc_accf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/uipc_accf.c wave=wave4 loc=311
export namespace pbsd::port::wave4::hbsd::src::sys::kern::uipc_accf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::uipc_accf
