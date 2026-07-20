export module pbsd.port.wave9.hbsd.src.include.ttyent;

module;
// Header bridge — replace #include of hbsd/src/include/ttyent.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/include/ttyent.h wave=wave9 loc=76
export namespace pbsd::port::wave9::hbsd::src::include::ttyent {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::include::ttyent
