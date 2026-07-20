export module pbsd.port.wave9.hbsd.src.tests.sys.fs.fusefs.fallocate;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/fs/fusefs/fallocate.cc
// void fallocate_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/fs/fusefs/fallocate.cc wave=wave9 loc=779
export namespace pbsd::port::wave9::hbsd::src::tests::sys::fs::fusefs::fallocate {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::fs::fusefs::fallocate
