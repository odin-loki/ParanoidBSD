export module pbsd.port.wave2.hbsd.src.usr_bin.users.users;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/users/users.cc
// void users_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/users/users.cc wave=wave2 loc=71
export namespace pbsd::port::wave2::hbsd::src::usr_bin::users::users {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::users::users
