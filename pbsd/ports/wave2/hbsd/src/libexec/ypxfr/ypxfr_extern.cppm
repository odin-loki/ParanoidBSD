export module pbsd.port.wave2.hbsd.src.libexec.ypxfr.ypxfr_extern;

module;
// Header bridge — replace #include of hbsd/src/libexec/ypxfr/ypxfr_extern.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/libexec/ypxfr/ypxfr_extern.h wave=wave2 loc=62
export namespace pbsd::port::wave2::hbsd::src::libexec::ypxfr::ypxfr_extern {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::libexec::ypxfr::ypxfr_extern
