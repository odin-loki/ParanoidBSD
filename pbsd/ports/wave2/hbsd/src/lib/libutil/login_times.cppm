export module pbsd.port.wave2.hbsd.src.lib.libutil.login_times;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libutil/login_times.c
// void login_times_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libutil/login_times.c wave=wave2 loc=159
export namespace pbsd::port::wave2::hbsd::src::lib::libutil::login_times {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libutil::login_times
