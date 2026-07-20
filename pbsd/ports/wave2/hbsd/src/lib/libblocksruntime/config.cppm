export module pbsd.port.wave2.hbsd.src.lib.libblocksruntime.config;

module;
// Header bridge — replace #include of hbsd/src/lib/libblocksruntime/config.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libblocksruntime/config.h wave=wave2 loc=13
export namespace pbsd::port::wave2::hbsd::src::lib::libblocksruntime::config {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libblocksruntime::config
