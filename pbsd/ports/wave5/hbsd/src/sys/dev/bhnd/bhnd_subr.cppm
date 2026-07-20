export module pbsd.port.wave5.hbsd.src.sys.dev.bhnd.bhnd_subr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/bhnd/bhnd_subr.c
// void bhnd_subr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/bhnd/bhnd_subr.c wave=wave5 loc=2320
export namespace pbsd::port::wave5::hbsd::src::sys::dev::bhnd::bhnd_subr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::bhnd::bhnd_subr
