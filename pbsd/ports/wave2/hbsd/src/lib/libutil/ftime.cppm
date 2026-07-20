export module pbsd.port.wave2.hbsd.src.lib.libutil.ftime;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libutil/ftime.c
// void ftime_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libutil/ftime.c wave=wave2 loc=53
export namespace pbsd::port::wave2::hbsd::src::lib::libutil::ftime {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libutil::ftime
