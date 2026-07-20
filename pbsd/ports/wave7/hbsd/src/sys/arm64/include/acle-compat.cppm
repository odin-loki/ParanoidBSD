export module pbsd.port.wave7.hbsd.src.sys.arm64.include.acle_compat;

module;
// Header bridge — replace #include of hbsd/src/sys/arm64/include/acle-compat.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/arm64/include/acle-compat.h wave=wave7 loc=5
export namespace pbsd::port::wave7::hbsd::src::sys::arm64::include::acle_compat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::arm64::include::acle_compat
