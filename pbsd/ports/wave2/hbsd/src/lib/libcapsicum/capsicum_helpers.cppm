export module pbsd.port.wave2.hbsd.src.lib.libcapsicum.capsicum_helpers;

module;
// Header bridge — replace #include of hbsd/src/lib/libcapsicum/capsicum_helpers.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

export import pbsd.userland.capsicum;
export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libcapsicum/capsicum_helpers.h wave=wave2 loc=205
export namespace pbsd::port::wave2::hbsd::src::lib::libcapsicum::capsicum_helpers {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libcapsicum::capsicum_helpers
