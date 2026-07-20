export module pbsd.port.wave2.hbsd.src.lib.libutil.flopen;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libutil/flopen.c
// void flopen_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libutil/flopen.c wave=wave2 loc=145
export namespace pbsd::port::wave2::hbsd::src::lib::libutil::flopen {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libutil::flopen
