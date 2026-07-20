export module pbsd.port.wave4.hbsd.src.sys.compat.linux.linux_elf64;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linux/linux_elf64.c
// void linux_elf64_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linux/linux_elf64.c wave=wave4 loc=35
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_elf64 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_elf64
