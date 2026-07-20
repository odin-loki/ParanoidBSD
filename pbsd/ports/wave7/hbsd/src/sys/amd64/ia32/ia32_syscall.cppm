export module pbsd.port.wave7.hbsd.src.sys.amd64.ia32.ia32_syscall;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/amd64/ia32/ia32_syscall.c
// void ia32_syscall_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/amd64/ia32/ia32_syscall.c wave=wave7 loc=299
export namespace pbsd::port::wave7::hbsd::src::sys::amd64::ia32::ia32_syscall {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::amd64::ia32::ia32_syscall
