export module pbsd.port.wave9.hbsd.src.tests.sys.kern.copy_file_range;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/kern/copy_file_range.c
// void copy_file_range_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/kern/copy_file_range.c wave=wave9 loc=231
export namespace pbsd::port::wave9::hbsd::src::tests::sys::kern::copy_file_range {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::kern::copy_file_range
