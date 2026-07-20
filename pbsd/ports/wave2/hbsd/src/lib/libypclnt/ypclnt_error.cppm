export module pbsd.port.wave2.hbsd.src.lib.libypclnt.ypclnt_error;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libypclnt/ypclnt_error.c
// void ypclnt_error_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libypclnt/ypclnt_error.c wave=wave2 loc=59
export namespace pbsd::port::wave2::hbsd::src::lib::libypclnt::ypclnt_error {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libypclnt::ypclnt_error
