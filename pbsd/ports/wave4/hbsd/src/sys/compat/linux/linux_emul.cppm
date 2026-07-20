export module pbsd.port.wave4.hbsd.src.sys.compat.linux.linux_emul;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linux/linux_emul.c
// void linux_emul_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linux/linux_emul.c wave=wave4 loc=353
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_emul {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_emul
