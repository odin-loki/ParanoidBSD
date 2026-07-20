export module pbsd.port.wave9.hbsd.src.tests.sys.arch.amd64.int0x80;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/arch/amd64/int0x80.c
// void int0x80_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/arch/amd64/int0x80.c wave=wave9 loc=95
export namespace pbsd::port::wave9::hbsd::src::tests::sys::arch::amd64::int0x80 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::arch::amd64::int0x80
