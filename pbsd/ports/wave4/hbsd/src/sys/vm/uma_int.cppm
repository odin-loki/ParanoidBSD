export module pbsd.port.wave4.hbsd.src.sys.vm.uma_int;

module;
// Header bridge — replace #include of hbsd/src/sys/vm/uma_int.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/vm/uma_int.h wave=wave4 loc=680
export namespace pbsd::port::wave4::hbsd::src::sys::vm::uma_int {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::vm::uma_int
