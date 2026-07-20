export module pbsd.port.wave4.hbsd.src.sys.sys._uexterror;

module;
// Header bridge — replace #include of hbsd/src/sys/sys/_uexterror.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/sys/_uexterror.h wave=wave4 loc=29
export namespace pbsd::port::wave4::hbsd::src::sys::sys::_uexterror {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::sys::_uexterror
