export module pbsd.port.wave9.hbsd.src.contrib.libfido2.regress.mock;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libfido2/regress/mock.c
// void mock_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libfido2/regress/mock.c wave=wave9 loc=182
export namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::regress::mock {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::regress::mock
