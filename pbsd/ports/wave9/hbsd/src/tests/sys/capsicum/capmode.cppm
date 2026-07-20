export module pbsd.port.wave9.hbsd.src.tests.sys.capsicum.capmode;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/capsicum/capmode.cc
// void capmode_init(void);
}

export import pbsd.userland.capsicum;
export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/capsicum/capmode.cc wave=wave9 loc=777
export namespace pbsd::port::wave9::hbsd::src::tests::sys::capsicum::capmode {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::capsicum::capmode
