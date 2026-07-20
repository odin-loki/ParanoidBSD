export module pbsd.port.wave9.hbsd.src.tests.sys.fs.fusefs.mkdir;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/fs/fusefs/mkdir.cc
// void mkdir_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/fs/fusefs/mkdir.cc wave=wave9 loc=274
export namespace pbsd::port::wave9::hbsd::src::tests::sys::fs::fusefs::mkdir {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::fs::fusefs::mkdir
