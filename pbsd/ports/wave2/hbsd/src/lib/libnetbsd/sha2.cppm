export module pbsd.port.wave2.hbsd.src.lib.libnetbsd.sha2;

module;
// Header bridge — replace #include of hbsd/src/lib/libnetbsd/sha2.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libnetbsd/sha2.h wave=wave2 loc=41
export namespace pbsd::port::wave2::hbsd::src::lib::libnetbsd::sha2 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libnetbsd::sha2
