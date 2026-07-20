export module pbsd.port.wave9.hbsd.src.contrib.netbsd_tests.rump.kernspace.tsleep;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/netbsd-tests/rump/kernspace/tsleep.c
// void tsleep_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/netbsd-tests/rump/kernspace/tsleep.c wave=wave9 loc=100
export namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::rump::kernspace::tsleep {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::netbsd_tests::rump::kernspace::tsleep
