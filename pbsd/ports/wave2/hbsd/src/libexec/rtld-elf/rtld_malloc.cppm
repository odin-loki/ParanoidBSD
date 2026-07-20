export module pbsd.port.wave2.hbsd.src.libexec.rtld_elf.rtld_malloc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/libexec/rtld-elf/rtld_malloc.c
// void rtld_malloc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/libexec/rtld-elf/rtld_malloc.c wave=wave2 loc=322
export namespace pbsd::port::wave2::hbsd::src::libexec::rtld_elf::rtld_malloc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::libexec::rtld_elf::rtld_malloc
