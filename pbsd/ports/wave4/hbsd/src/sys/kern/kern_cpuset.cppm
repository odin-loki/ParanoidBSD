export module pbsd.port.wave4.hbsd.src.sys.kern.kern_cpuset;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/kern_cpuset.c
// void kern_cpuset_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/kern_cpuset.c wave=wave4 loc=2635
export namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_cpuset {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_cpuset
