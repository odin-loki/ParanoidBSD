export module pbsd.port.wave9.hbsd.src.include.inttypes;

module;
// Header bridge — replace #include of hbsd/src/include/inttypes.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/include/inttypes.h wave=wave9 loc=62
export namespace pbsd::port::wave9::hbsd::src::include::inttypes {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::include::inttypes
