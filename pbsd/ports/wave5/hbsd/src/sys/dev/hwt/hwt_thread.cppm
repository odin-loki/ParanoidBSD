export module pbsd.port.wave5.hbsd.src.sys.dev.hwt.hwt_thread;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/hwt/hwt_thread.c
// void hwt_thread_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/hwt/hwt_thread.c wave=wave5 loc=162
export namespace pbsd::port::wave5::hbsd::src::sys::dev::hwt::hwt_thread {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::hwt::hwt_thread
