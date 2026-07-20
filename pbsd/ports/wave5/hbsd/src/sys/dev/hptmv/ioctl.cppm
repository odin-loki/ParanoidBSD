export module pbsd.port.wave5.hbsd.src.sys.dev.hptmv.ioctl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/hptmv/ioctl.c
// void ioctl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/hptmv/ioctl.c wave=wave5 loc=949
export namespace pbsd::port::wave5::hbsd::src::sys::dev::hptmv::ioctl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::hptmv::ioctl
