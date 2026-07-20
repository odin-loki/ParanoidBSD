export module pbsd.port.wave5.hbsd.src.sys.dev.ice.ice_iov;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ice/ice_iov.c
// void ice_iov_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ice/ice_iov.c wave=wave5 loc=1856
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ice::ice_iov {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ice::ice_iov
