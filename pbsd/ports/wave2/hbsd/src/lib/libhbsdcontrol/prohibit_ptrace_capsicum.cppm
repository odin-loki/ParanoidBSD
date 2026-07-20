export module pbsd.port.wave2.hbsd.src.lib.libhbsdcontrol.prohibit_ptrace_capsicum;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libhbsdcontrol/prohibit_ptrace_capsicum.c
// void prohibit_ptrace_capsicum_init(void);
}

export import pbsd.userland.capsicum;
export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libhbsdcontrol/prohibit_ptrace_capsicum.c wave=wave2 loc=315
export namespace pbsd::port::wave2::hbsd::src::lib::libhbsdcontrol::prohibit_ptrace_capsicum {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libhbsdcontrol::prohibit_ptrace_capsicum
