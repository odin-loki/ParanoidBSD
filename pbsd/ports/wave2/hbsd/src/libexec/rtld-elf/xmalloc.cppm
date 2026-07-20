export module pbsd.port.wave2.hbsd.src.libexec.rtld_elf.xmalloc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/libexec/rtld-elf/xmalloc.c
// void xmalloc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/libexec/rtld-elf/xmalloc.c wave=wave2 loc=162
export namespace pbsd::port::wave2::hbsd::src::libexec::rtld_elf::xmalloc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::libexec::rtld_elf::xmalloc
