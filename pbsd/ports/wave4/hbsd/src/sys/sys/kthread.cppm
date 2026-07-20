export module pbsd.port.wave4.hbsd.src.sys.sys.kthread;

module;
// Header bridge — replace #include of hbsd/src/sys/sys/kthread.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/sys/kthread.h wave=wave4 loc=78
export namespace pbsd::port::wave4::hbsd::src::sys::sys::kthread {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::sys::kthread
