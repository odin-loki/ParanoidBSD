export module pbsd.port.wave2.hbsd.src.libexec.rtld_elf.rtld_libc.libc_private;

module;
// Header bridge — replace #include of hbsd/src/libexec/rtld-elf/rtld-libc/libc_private.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/libexec/rtld-elf/rtld-libc/libc_private.h wave=wave2 loc=37
export namespace pbsd::port::wave2::hbsd::src::libexec::rtld_elf::rtld_libc::libc_private {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::libexec::rtld_elf::rtld_libc::libc_private
