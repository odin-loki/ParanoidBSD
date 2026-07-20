export module pbsd.port.wave9.hbsd.src.include.arpa.tftp;

module;
// Header bridge — replace #include of hbsd/src/include/arpa/tftp.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/include/arpa/tftp.h wave=wave9 loc=80
export namespace pbsd::port::wave9::hbsd::src::include::arpa::tftp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::include::arpa::tftp
