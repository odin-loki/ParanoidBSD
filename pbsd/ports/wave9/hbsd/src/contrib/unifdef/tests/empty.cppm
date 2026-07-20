export module pbsd.port.wave9.hbsd.src.contrib.unifdef.tests.empty;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/unifdef/tests/empty.c
// void empty_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/unifdef/tests/empty.c wave=wave9 loc=0
export namespace pbsd::port::wave9::hbsd::src::contrib::unifdef::tests::empty {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::unifdef::tests::empty
