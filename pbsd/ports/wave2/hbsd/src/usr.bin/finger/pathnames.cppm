export module pbsd.port.wave2.hbsd.src.usr_bin.finger.pathnames;

module;
// Header bridge — replace #include of hbsd/src/usr.bin/finger/pathnames.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/finger/pathnames.h wave=wave2 loc=41
export namespace pbsd::port::wave2::hbsd::src::usr_bin::finger::pathnames {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::finger::pathnames
