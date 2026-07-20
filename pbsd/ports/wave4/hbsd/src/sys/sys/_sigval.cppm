export module pbsd.port.wave4.hbsd.src.sys.sys._sigval;

module;
// Header bridge — replace #include of hbsd/src/sys/sys/_sigval.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/sys/_sigval.h wave=wave4 loc=61
export namespace pbsd::port::wave4::hbsd::src::sys::sys::_sigval {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::sys::_sigval
