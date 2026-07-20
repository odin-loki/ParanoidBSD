export module pbsd.port.wave7.hbsd.src.sys.amd64.ia32.ia32_signal;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/amd64/ia32/ia32_signal.c
// void ia32_signal_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/amd64/ia32/ia32_signal.c wave=wave7 loc=990
export namespace pbsd::port::wave7::hbsd::src::sys::amd64::ia32::ia32_signal {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::amd64::ia32::ia32_signal
