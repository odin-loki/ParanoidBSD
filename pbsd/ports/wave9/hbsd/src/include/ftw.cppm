export module pbsd.port.wave9.hbsd.src.include.ftw;

module;
// Header bridge — replace #include of hbsd/src/include/ftw.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/include/ftw.h wave=wave9 loc=62
export namespace pbsd::port::wave9::hbsd::src::include::ftw {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::include::ftw
