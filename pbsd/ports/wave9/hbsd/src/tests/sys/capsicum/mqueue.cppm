export module pbsd.port.wave9.hbsd.src.tests.sys.capsicum.mqueue;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/capsicum/mqueue.cc
// void mqueue_init(void);
}

export import pbsd.userland.capsicum;
export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/capsicum/mqueue.cc wave=wave9 loc=99
export namespace pbsd::port::wave9::hbsd::src::tests::sys::capsicum::mqueue {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::capsicum::mqueue
