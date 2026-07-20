export module pbsd.port.wave4.hbsd.src.sys.compat.linuxkpi.dummy.include.asm.msr_index;

module;
// Header bridge — replace #include of hbsd/src/sys/compat/linuxkpi/dummy/include/asm/msr-index.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linuxkpi/dummy/include/asm/msr-index.h wave=wave4 loc=0
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::dummy::include::asm::msr_index {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::dummy::include::asm::msr_index
