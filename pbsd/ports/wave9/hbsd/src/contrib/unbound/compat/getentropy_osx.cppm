export module pbsd.port.wave9.hbsd.src.contrib.unbound.compat.getentropy_osx;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/unbound/compat/getentropy_osx.c
// void getentropy_osx_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/unbound/compat/getentropy_osx.c wave=wave9 loc=417
export namespace pbsd::port::wave9::hbsd::src::contrib::unbound::compat::getentropy_osx {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::unbound::compat::getentropy_osx
