export module pbsd.port.wave4.hbsd.src.sys.sys.elf;

module;
// Header bridge — replace #include of hbsd/src/sys/sys/elf.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/sys/elf.h wave=wave4 loc=41
export namespace pbsd::port::wave4::hbsd::src::sys::sys::elf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::sys::elf
