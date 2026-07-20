export module pbsd.port.wave2.hbsd.src.lib.csu.aarch64.crt;

module;
// Header bridge — replace #include of hbsd/src/lib/csu/aarch64/crt.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/csu/aarch64/crt.h wave=wave2 loc=1
export namespace pbsd::port::wave2::hbsd::src::lib::csu::aarch64::crt {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::csu::aarch64::crt
