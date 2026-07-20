export module pbsd.port.wave9.hbsd.src.include.getopt;

module;
// Header bridge — replace #include of hbsd/src/include/getopt.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/include/getopt.h wave=wave9 loc=79
export namespace pbsd::port::wave9::hbsd::src::include::getopt {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::include::getopt
