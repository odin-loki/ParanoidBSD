export module pbsd.port.wave2.hbsd.src.usr_bin.at.privs;

module;
// Header bridge — replace #include of hbsd/src/usr.bin/at/privs.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/at/privs.h wave=wave2 loc=107
export namespace pbsd::port::wave2::hbsd::src::usr_bin::at::privs {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::at::privs
