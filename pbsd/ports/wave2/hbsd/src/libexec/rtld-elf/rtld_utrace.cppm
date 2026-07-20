export module pbsd.port.wave2.hbsd.src.libexec.rtld_elf.rtld_utrace;

module;
// Header bridge — replace #include of hbsd/src/libexec/rtld-elf/rtld_utrace.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/libexec/rtld-elf/rtld_utrace.h wave=wave2 loc=61
export namespace pbsd::port::wave2::hbsd::src::libexec::rtld_elf::rtld_utrace {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::libexec::rtld_elf::rtld_utrace
