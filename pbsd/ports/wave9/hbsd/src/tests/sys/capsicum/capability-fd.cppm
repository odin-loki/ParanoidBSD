export module pbsd.port.wave9.hbsd.src.tests.sys.capsicum.capability_fd;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/capsicum/capability-fd.cc
// void capability-fd_init(void);
}

export import pbsd.userland.capsicum;
export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/capsicum/capability-fd.cc wave=wave9 loc=1359
export namespace pbsd::port::wave9::hbsd::src::tests::sys::capsicum::capability_fd {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::capsicum::capability_fd
