export module pbsd.port.wave9.hbsd.src.tests.sys.kern.sched_affinity;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/kern/sched_affinity.c
// void sched_affinity_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/kern/sched_affinity.c wave=wave9 loc=275
export namespace pbsd::port::wave9::hbsd::src::tests::sys::kern::sched_affinity {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::kern::sched_affinity
