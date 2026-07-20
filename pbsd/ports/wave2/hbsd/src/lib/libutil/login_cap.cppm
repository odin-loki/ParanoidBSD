export module pbsd.port.wave2.hbsd.src.lib.libutil.login_cap;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libutil/login_cap.c
// void login_cap_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libutil/login_cap.c wave=wave2 loc=959
export namespace pbsd::port::wave2::hbsd::src::lib::libutil::login_cap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libutil::login_cap
