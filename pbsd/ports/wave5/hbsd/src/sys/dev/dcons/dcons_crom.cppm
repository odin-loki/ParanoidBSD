export module pbsd.port.wave5.hbsd.src.sys.dev.dcons.dcons_crom;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/dcons/dcons_crom.c
// void dcons_crom_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/dcons/dcons_crom.c wave=wave5 loc=269
export namespace pbsd::port::wave5::hbsd::src::sys::dev::dcons::dcons_crom {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::dcons::dcons_crom
