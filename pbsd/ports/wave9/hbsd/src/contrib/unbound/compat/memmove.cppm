export module pbsd.port.wave9.hbsd.src.contrib.unbound.compat.memmove;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/unbound/compat/memmove.c
// void memmove_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/unbound/compat/memmove.c wave=wave9 loc=43
export namespace pbsd::port::wave9::hbsd::src::contrib::unbound::compat::memmove {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::unbound::compat::memmove
