export module pbsd.port.wave4.hbsd.src.sys.compat.linux.linux_elf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linux/linux_elf.c
// void linux_elf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linux/linux_elf.c wave=wave4 loc=545
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_elf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_elf
