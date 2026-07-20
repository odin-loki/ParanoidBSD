export module pbsd.port.wave4.hbsd.src.sys.sys._null;

module;
// Header bridge — replace #include of hbsd/src/sys/sys/_null.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/sys/_null.h wave=wave4 loc=45
export namespace pbsd::port::wave4::hbsd::src::sys::sys::_null {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::sys::_null
