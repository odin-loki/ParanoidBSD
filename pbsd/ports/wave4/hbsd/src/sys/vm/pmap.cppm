export module pbsd.port.wave4.hbsd.src.sys.vm.pmap;

module;
// Header bridge — replace #include of hbsd/src/sys/vm/pmap.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/vm/pmap.h wave=wave4 loc=174
export namespace pbsd::port::wave4::hbsd::src::sys::vm::pmap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::vm::pmap
