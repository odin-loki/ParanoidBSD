export module pbsd.port.wave4.hbsd.src.sys.sys._param;

module;
// Header bridge — replace #include of hbsd/src/sys/sys/_param.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/sys/_param.h wave=wave4 loc=28
export namespace pbsd::port::wave4::hbsd::src::sys::sys::_param {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::sys::_param
