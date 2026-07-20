export module pbsd.port.wave9.hbsd.src.contrib.unbound.compat.getentropy_linux;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/unbound/compat/getentropy_linux.c
// void getentropy_linux_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/unbound/compat/getentropy_linux.c wave=wave9 loc=559
export namespace pbsd::port::wave9::hbsd::src::contrib::unbound::compat::getentropy_linux {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::unbound::compat::getentropy_linux
