export module pbsd.port.wave9.hbsd.src.include.varargs;

module;
// Header bridge — replace #include of hbsd/src/include/varargs.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/include/varargs.h wave=wave9 loc=35
export namespace pbsd::port::wave9::hbsd::src::include::varargs {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::include::varargs
