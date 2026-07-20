export module pbsd.port.wave9.hbsd.src.include.mqueue;

module;
// Header bridge — replace #include of hbsd/src/include/mqueue.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/include/mqueue.h wave=wave9 loc=57
export namespace pbsd::port::wave9::hbsd::src::include::mqueue {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::include::mqueue
