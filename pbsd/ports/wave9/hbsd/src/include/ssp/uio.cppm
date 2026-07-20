export module pbsd.port.wave9.hbsd.src.include.ssp.uio;

module;
// Header bridge — replace #include of hbsd/src/include/ssp/uio.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/include/ssp/uio.h wave=wave9 loc=53
export namespace pbsd::port::wave9::hbsd::src::include::ssp::uio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::include::ssp::uio
