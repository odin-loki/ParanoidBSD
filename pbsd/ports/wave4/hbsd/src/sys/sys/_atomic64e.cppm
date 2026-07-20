export module pbsd.port.wave4.hbsd.src.sys.sys._atomic64e;

module;
// Header bridge — replace #include of hbsd/src/sys/sys/_atomic64e.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/sys/_atomic64e.h wave=wave4 loc=77
export namespace pbsd::port::wave4::hbsd::src::sys::sys::_atomic64e {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::sys::_atomic64e
