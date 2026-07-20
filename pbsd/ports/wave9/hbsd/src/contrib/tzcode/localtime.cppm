export module pbsd.port.wave9.hbsd.src.contrib.tzcode.localtime;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tzcode/localtime.c
// void localtime_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tzcode/localtime.c wave=wave9 loc=3176
export namespace pbsd::port::wave9::hbsd::src::contrib::tzcode::localtime {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tzcode::localtime
