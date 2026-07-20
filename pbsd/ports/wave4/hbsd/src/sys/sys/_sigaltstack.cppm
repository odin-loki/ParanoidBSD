export module pbsd.port.wave4.hbsd.src.sys.sys._sigaltstack;

module;
// Header bridge — replace #include of hbsd/src/sys/sys/_sigaltstack.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/sys/_sigaltstack.h wave=wave4 loc=65
export namespace pbsd::port::wave4::hbsd::src::sys::sys::_sigaltstack {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::sys::_sigaltstack
